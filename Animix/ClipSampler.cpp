#include "ClipSampler.h"

#include "AnimationClip.h"
#include "AnimationEngine.h"


namespace Animix
{
	ClipSampler::ClipSampler(const AnimationClip* clip)
		: m_Clip(clip)
	{
	}

	void ClipSampler::PlayFromStart()
	{
		m_LocalTimer = 0.0f;
	}


	void ClipSampler::Tick(float timeScale)
	{
		// Perform tick
		if (m_LastTickIndex != g_AnimixEngine->GetTickIndex())
		{
			const float netScale = m_PlaybackSpeed * timeScale;
			m_LocalTimer += g_AnimixEngine->GetDeltaTime() * netScale;
			m_LastTickIndex = g_AnimixEngine->GetTickIndex();
		}

		if (m_LocalTimer > GetDuration())
		{
			if (m_Looping)
				m_LocalTimer = fmodf(m_LocalTimer, GetDuration());
			// if not looping, animation clip will automatically clamp at the end of the animation data
		}
	}

	float ClipSampler::GetCurrentSampleTime() const
	{
		return m_LocalTimer;
	}

	float ClipSampler::GetDuration() const
	{
		return m_Clip->GetDuration() * m_PlaybackSpeed;
	}

}
