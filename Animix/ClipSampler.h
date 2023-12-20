#pragma once

#include <cstdint>


namespace Animix
{
	class AnimationClip;


	/*
	 * A collection of properties and methods to sample and animation clip.
	 * Encompasses sampling logic and operations.
	 */
	class ClipSampler
	{
	public:
		ClipSampler(const AnimationClip* clip);

		void Tick(float timeScale);
		float GetCurrentSampleTime() const;

		float GetDuration() const;

		// Manipulation operations
		void PlayFromStart();

		// Getters and setters
		inline const AnimationClip* GetClip() const { return m_Clip; }
		inline void SetClip(const AnimationClip* clip) { m_Clip = clip; }

		inline float GetPlaybackSpeed() const { return m_PlaybackSpeed; }
		inline void SetPlaybackSpeed(float playbackSpeed) { m_PlaybackSpeed = playbackSpeed; }
		inline bool GetLooping() const { return m_Looping; }
		inline void SetLooping(bool looping) { m_Looping = looping; }

	private:
		const AnimationClip* m_Clip = nullptr;	// The clip is required for duration information

		// Playback properties
		float m_PlaybackSpeed = 1.0f;
		bool m_Looping = false;

		// The local timer
		float m_LocalTimer = 0.0f;
		uint64_t m_LastTickIndex = 0u;
	};

}
