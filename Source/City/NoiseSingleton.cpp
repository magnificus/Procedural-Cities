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
	return raw_noise_2d(noiseScale * (x + xOffset), noiseScale*(y + yOffset));
}