#pragma once
#include <math.h>
#include <vector>



class Delay {
public:
	//Delay(int bufferSize) : m_buffer(bufferSize) { m_bufferSize = bufferSize; }
	Delay() { m_bufferSize = 44100; }
	float ProcessSample(float input);

	void SetDelayInSmples(float delay) { p_delayInSamples = delay; }
	void SetDelayInMs(float ms) { p_delayInSamples = 44.1f * ms; }

	void SetFeedback(float feed) { p_feedback = feed; }

private:
	float p_delayInSamples;
	float p_feedback;

	float m_buffer[44100];
	int m_bufferSize = 44100;

	float m_readPointer;
	int m_writePointer;

	float m_smoothedDelayInSamples;

};


float Delay::ProcessSample(float input) 
{
	m_smoothedDelayInSamples = 0.999f * m_smoothedDelayInSamples + 0.0001f * p_delayInSamples;

	m_readPointer = m_writePointer - static_cast<int>(m_smoothedDelayInSamples);
	while (static_cast<int>(m_readPointer) < 0) m_readPointer += m_smoothedDelayInSamples;

	m_buffer[m_writePointer] = input;

	m_writePointer++;
	if (m_writePointer > m_bufferSize) m_writePointer = 0;

	return m_buffer[static_cast<int>(m_readPointer)];
}