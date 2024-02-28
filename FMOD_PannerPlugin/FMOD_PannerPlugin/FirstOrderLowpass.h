#pragma once
#include <math.h>



class FirstOrderLowpass {
public:
	FirstOrderLowpass() 
	{
		m_cutoffFrequency = 20000.0f;
		m_buf = 0.0f;
	}

	void SetCutoffFrequencyNorm(float fc) { m_cutoffFrequency = fc; }
	void SetCutoffFrequency(float fc) { m_cutoffFrequency = fc/20000.0f; }

	float GetCutoffFrequency() { return m_cutoffFrequency; }

	float ProcessSample(float input);

private:
	float m_cutoffFrequency;
	float m_buf;

};

float FirstOrderLowpass::ProcessSample(float input) {

	m_buf = (1 - m_cutoffFrequency) * m_buf + (m_cutoffFrequency) * input;

	return m_buf;
}