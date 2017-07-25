// Fill out your copyright notice in the Description page of Project Settings.

#include "City.h"
#include "NoiseSingleton.h"
#include "simplexnoise.h"

NoiseSingleton* NoiseSingleton::instance;


NoiseSingleton::NoiseSingleton()
{
}

NoiseSingleton::~NoiseSingleton()
{
}

float NoiseSingleton::noise(float x, float y, float noiseScale, float xOffset, float yOffset){
	if (!useTexture)
		return raw_noise_2d(noiseScale * (x + xOffset), noiseScale*(y + yOffset));
	else {

		FTexture2DMipMap* MyMipMap = &image->PlatformData->Mips[0];
		FByteBulkData* RawImageData = &MyMipMap->BulkData;
		FColor* FormatedImageData = static_cast<FColor*>(RawImageData->Lock(LOCK_READ_ONLY));
		uint32 TextureWidth = MyMipMap->SizeX, TextureHeight = MyMipMap->SizeY;
		FColor PixelColor;

		uint32 intX = (FMath::FloorToInt(x * noiseScale * noiseTextureScale));
		uint32 intY = (FMath::FloorToInt(y * noiseScale * noiseTextureScale));

		if (intX >= TextureWidth || intY >= TextureHeight) {
			RawImageData->Unlock();
			return 0.0f;
		}
		//if (intX < 0)
		//	intX = TextureWidth + intX;
		//if (intY < 0)
		//	intY = TextureHeight + intY;

		PixelColor = FormatedImageData[intY * TextureWidth + intX];
		RawImageData->Unlock();

		return (PixelColor.R / 255.0f);

	}
}

FVector NoiseSingleton::getStartSuggestion(float noiseScale) {
	if (!useTexture)
		return FVector(0, 0, 0);
	else {
		FTexture2DMipMap* MyMipMap = &image->PlatformData->Mips[0];
		FByteBulkData* RawImageData = &MyMipMap->BulkData;
		FColor* FormatedImageData = static_cast<FColor*>(RawImageData->Lock(LOCK_READ_ONLY));
		uint32 TextureWidth = MyMipMap->SizeX, TextureHeight = MyMipMap->SizeY;
		RawImageData->Unlock();

		UE_LOG(LogTemp, Warning, TEXT("start x,y: %f, %f"), (TextureWidth / (noiseTextureScale * noiseScale)), (TextureHeight / (noiseTextureScale * noiseScale)));
		return FVector((TextureWidth / (noiseTextureScale * noiseScale)) / 2, (TextureHeight / (noiseTextureScale * noiseScale)) / 2, 0);
	}
}