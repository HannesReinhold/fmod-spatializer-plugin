#pragma once
#include <math.h>
#include <vector>



template<int numChannels = 8, int numDiffusionSteps = 3>
struct DiffusionNetwork {
	int numChannels = 8;

	//std::array<Diffuser<numChannels>, numDiffusionSteps> diffusers;

	void Process(float* input) {
		for (int i = 0; i < numDiffusionSteps; i++) {
			//diffusers[i].Process(input);
		}
	}

	DiffusionNetwork() {

	}

};


template<int numChannels = 8>
struct Diffuser {

	void Process(float* input) {
		for (int i = 0; i < numChannels; i++) {
			input[i] *= 0.5f;
		}
	}

	Diffuser() {

	}
};


template<int numChannels = 8>
class FeedbackDelayNetwork {
public:
	void Process(float* input) {
		for (int i = 0; i < numChannels; i++) {
			input[i] *= 0.1f;
		}
	}

	FeedbackDelayNetwork() {

	}
};


template<int numChannels = 8, int diffusionSteps = 4>
class Reverb {
private:

	DiffusionNetwork<numChannels, diffusionSteps> diffuser;
	FeedbackDelayNetwork<numChannels> feedbackNetwork;

	float dry, wet;


public:
	void ProcessSample(float *inputBuffer)
	{
		diffuser.Process(inputBuffer);
		feedbackNetwork.Process(inputBuffer);

	};

	Reverb()
	{

	}

};



