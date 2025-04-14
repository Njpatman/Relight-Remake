modded class SCR_BaseInteractiveLightComponent
{
	protected RR_BaseLightData m_CurrentLightData;
	
	//------------------------------------------------------------------------------------------------
	override protected void TurnOn(notnull SCR_BaseInteractiveLightComponentClass componentData, bool skipTransition, bool playSound)
	{
		IEntity owner = GetOwner();
		
		// Play sound
		if (!System.IsConsoleApp())
		{
			if (playSound)
			{
				SoundComponent soundComponent = SoundComponent.Cast(owner.FindComponent(SoundComponent));
				if (soundComponent)
					soundComponent.SoundEvent(SCR_SoundEvent.SOUND_TURN_ON);
			}
			
			SignalsManagerComponent signalsManagerComponent = SignalsManagerComponent.Cast(owner.FindComponent(SignalsManagerComponent));
			if (signalsManagerComponent)
				signalsManagerComponent.SetSignalValue(signalsManagerComponent.AddOrFindSignal("Trigger"), 1);
		}
		
		if (!m_aLights)
			m_aLights = {};
		
		m_CurrentLightData = null;
		
		foreach (SCR_BaseLightData lightData : componentData.GetLightData())
		{
			m_CurrentLightData = RR_BaseLightData.Cast(lightData);
			
			if (!m_CurrentLightData)
				continue;
			
			vector mat[4];
			owner.GetWorldTransform(mat);
			
			vector lightDirection = m_CurrentLightData.GetLightConeDirection().Multiply3(mat).Normalized();
			vector pos = m_CurrentLightData.GetLightOffset().Multiply4(mat);
			
			GetGame().GetWorld().QueryEntitiesBySphere(pos, 0.85, ProcessEntity);
			
			LightEntity light = CreateLight(m_CurrentLightData, pos, lightDirection, LIGHT_EMISSIVITY_START);
			if (!light)
				continue;
			
			light.SetFlags(EntityFlags.PROXY);
			m_aLights.Insert(light);
		}
		
		m_bIsOn = true;
		m_bUpdate = true;
		m_CurrentLightData = null;
		
		// Skip transition phase of the light.
		if (skipTransition || !componentData.IsGradualLightningOn())
		{
			if (m_EmissiveMaterialComponent)
				m_EmissiveMaterialComponent.SetEmissiveMultiplier(MATERIAL_EMISSIVITY_ON);

			for (int i = 0, count = m_aLights.Count(); i < count; i++)
			{
				m_aLights[i].SetColor(Color.FromVector(componentData.GetLightData()[i].GetLightColor()), componentData.GetLightLV());
				m_aLights[i].SetDistanceAtt(componentData.GetDistanceAttenuation());
			}

			return;
		}
		
		m_fCurLV = LIGHT_EMISSIVITY_START;
		m_fCurEM = MATERIAL_EMISSIVITY_START;
		m_fLightEmisivityStep = componentData.GetLightLV() / ((MATERIAL_EMISSIVITY_ON - MATERIAL_EMISSIVITY_START) / MATERIAL_EMISSIVITY_STEP);
		
		SetEventMask(GetOwner(), EntityEvent.VISIBLE);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void TurnOff(notnull SCR_BaseInteractiveLightComponentClass componentData, bool playSound)
	{
		IEntity owner = GetOwner();
		
		// Play sound
		if (!System.IsConsoleApp())
		{
			SignalsManagerComponent signalsManagerComponent = SignalsManagerComponent.Cast(owner.FindComponent(SignalsManagerComponent));
						
			if (playSound)
			{
				SoundComponent soundComponent = SoundComponent.Cast(owner.FindComponent(SoundComponent));
				if (soundComponent)
					soundComponent.SoundEvent(SCR_SoundEvent.SOUND_TURN_OFF);
			}
				
			if (signalsManagerComponent)
				signalsManagerComponent.SetSignalValue(signalsManagerComponent.AddOrFindSignal("Trigger"), 0);
		}
		
		m_CurrentLightData = null;
		
		if (!componentData.GetLightData().IsEmpty())
		{
			foreach (SCR_BaseLightData lightData : componentData.GetLightData())
			{
				m_CurrentLightData = RR_BaseLightData.Cast(lightData);
				
				RemoveLights();
				
				vector mat[4];
				owner.GetWorldTransform(mat);
				
				vector pos = m_CurrentLightData.GetLightOffset().Multiply4(mat);
				GetGame().GetWorld().QueryEntitiesBySphere(pos, 0.85, ProcessEntity);
			};
			
			m_CurrentLightData = null;
				
			if (m_EmissiveMaterialComponent)
				m_EmissiveMaterialComponent.SetEmissiveMultiplier(LIGHT_EMISSIVITY_OFF);
		}
			
		ClearEventMask(GetOwner(), EntityEvent.VISIBLE);
		m_bIsOn = false;
		m_bUpdate = false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool ProcessEntity(IEntity processEntity)
	{
		if (m_CurrentLightData && m_CurrentLightData.m_rLightToChangeXOB == processEntity.GetVObject().GetResourceName()) 
		{
			SetLightMaterial(processEntity);
			return false;
		}

		return true;
	}

	protected void SetLightMaterial(IEntity entity)
	{
		VObject mesh = entity.GetVObject();
		if (mesh)
		{
			string remap;
			string materials[256];
			int numMats = mesh.GetMaterials(materials);
			for (int i = 0; i < numMats; i++)
			{
				if(i == m_CurrentLightData.m_iLightToChangeXOBMaterialNumber)
				{
					if(!m_bIsOn)
						remap += string.Format("$remap '%1' '%2';", materials[i], m_CurrentLightData.m_rLightToChangeOnXOBMaterial);
				 	else 
						remap += string.Format("$remap '%1' '%2';", materials[i], m_CurrentLightData.m_rLightToChangeOffXOBMaterial);
					break;
				};
			}
			entity.SetObject(mesh, remap);
		}
	}
}