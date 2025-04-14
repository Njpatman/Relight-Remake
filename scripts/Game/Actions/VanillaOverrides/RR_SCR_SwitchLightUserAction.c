modded class SCR_SwitchLightUserAction : ScriptedUserAction
{
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{	
		if (m_LightComp)
		{
			m_LightComp.ToggleLight(!m_LightComp.IsOn());
			PlaySound(pOwnerEntity, m_LightComp.IsOn());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void PlaySound(IEntity ownerEntity, bool lightsState)
	{
		// Sound
		SoundComponent soundComponent = SoundComponent.Cast(ownerEntity.FindComponent(SoundComponent));
		if (!soundComponent)
			return;
		
		vector offset;
		if (GetLightAudioPos(offset))
		{
			if (lightsState)
				soundComponent.SoundEventOffset(SCR_SoundEvent.SOUND_FLASHLIGHT_ON, offset);
			else
				soundComponent.SoundEventOffset(SCR_SoundEvent.SOUND_FLASHLIGHT_OFF, offset);
		}
		else
		{
			if (lightsState)
				soundComponent.SoundEvent(SCR_SoundEvent.SOUND_FLASHLIGHT_ON);
			else
				soundComponent.SoundEvent(SCR_SoundEvent.SOUND_FLASHLIGHT_OFF);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetLightAudioPos(out vector pos)
	{
		UserActionContext userActionContext = GetActiveContext();
		if (!userActionContext)
		{
			return false;
		}
		else
		{
			pos = GetOwner().CoordToLocal(userActionContext.GetOrigin());
			return true;
		}
	}
}