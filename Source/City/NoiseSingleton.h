// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

/**
 * 
 */
class CITY_API NoiseSingleton
{
private:
	NoiseSingleton();// {}
	static NoiseSingleton* instance;
	UTexture2D* image;

public:

	static NoiseSingleton* getInstance() {
		if (instance == nullptr)
			instance = new NoiseSingleton();
		return instance;
	}

	float noise(float x, float y, float noiseScale = 1.0f, float xOffset = 0, float yOffset = 0);
	bool useTexture = false;

	void setUseTexture(UTexture2D* inImage) {
		useTexture = true;
		this->image = inImage;
	}
	~NoiseSingleton();

	
};
