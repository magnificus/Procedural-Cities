// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

/**
 * 
 */
class CITY_API NoiseSingleton
{
	//static float noiseTextureScale;
	//static float noiseScale;
private:
	NoiseSingleton();
	static NoiseSingleton* instance;
	UTexture2D* image;

public:
	float noiseScale = 0.0;
	float noiseTextureScale = 0.0;
	float xOffset = 0;
	float yOffset = 0;
	void setNoiseScale(float inScale) { noiseScale = inScale; }

	static NoiseSingleton* getInstance() {
		if (instance == nullptr)
			instance = new NoiseSingleton();
		return instance;
	}

	float noise(float x, float y);
	FVector getStartSuggestion();
	bool useTexture = false;

	void setUseTexture(UTexture2D* inImage, float scale) {
		useTexture = true;
		this->image = inImage;
		this->noiseTextureScale = scale;
	}
	~NoiseSingleton();

	
};
