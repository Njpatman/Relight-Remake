modded class SCR_BaseInteractiveLightComponent
{
	protected RR_BaseLightData m_CurrentLightData;
	
	const static ref TStringArray m_aSwitches ={
		"{B41428EDBCF99A79}Assets/Structures/BuildingsParts/Electrical/LightSwitch/LightSwitch_01.xob",
		"{D055B781E81BED87}Assets/Structures/BuildingsParts/Electrical/LightSwitch/LightSwitch_02.xob",
		"{FE85810785DAD54A}Assets/Structures/BuildingsParts/Electrical/LightSwitch/LightSwitch_03.xob",
		"{BEB5DC297301718A}Assets/Structures/BuildingsParts/Electrical/LightSwitch/LightSwitch_04.xob"
	};
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		if(m_aSwitches.Contains(owner.GetVObject().GetResourceName()) && GetGame().InPlayMode())
		{
			SCR_BaseInteractiveLightComponentClass componentData = SCR_BaseInteractiveLightComponentClass.Cast(GetComponentData(owner));
			array<ref SCR_BaseLightData> components = componentData.GetLightData();
			
			if(components.IsEmpty())
				SCR_Global.SetMaterial(owner, "{0A94C84B94134E73}Assets/InvisibiltyGoesSoHard.emat");
		};
	}
	
	override void ToggleLight(bool turnOn, bool skipTransition = false, bool playSound = true)
	{
		if (m_bIsOn == turnOn || !GetGame().InPlayMode())
			return;
		
		GetGame().GetCallqueue().CallLater(ToggleLightDelayed, 50, false, turnOn, skipTransition, playSound)
	}
	
	void ToggleLightDelayed(bool turnOn, bool skipTransition = false, bool playSound = true)
	{
		SCR_BaseInteractiveLightComponentClass componentData = SCR_BaseInteractiveLightComponentClass.Cast(GetComponentData(GetOwner()));
		if (!componentData)
			return;
		
		if (turnOn)
			TurnOn(componentData, skipTransition, playSound);
		else
			TurnOff(componentData, playSound);
	}
	
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
			if (!lightData)
				continue;
			
			vector mat[4];
			owner.GetWorldTransform(mat);
			
			vector lightDirection = lightData.GetLightConeDirection().Multiply3(mat).Normalized();
			vector pos = lightData.GetLightOffset().Multiply4(mat);
			
			LightEntity light = CreateLight(lightData, pos, lightDirection, LIGHT_EMISSIVITY_START);
			if (!light)
				continue;
			
			light.SetFlags(EntityFlags.PROXY);
			m_aLights.Insert(light);
			
			m_CurrentLightData = RR_BaseLightData.Cast(lightData);
			
			if (m_CurrentLightData)
			{
				vector lightPos = m_CurrentLightData.GetLightOffset().Multiply4(mat);
				
				if(m_CurrentLightData.m_rLightToChangeOnXOBMaterial && m_CurrentLightData.m_rLightToChangeOffXOBMaterial && m_CurrentLightData.m_rLightToChangeXOB)
					GetGame().GetWorld().QueryEntitiesBySphere(lightPos, 1, ProcessEntity);
			};
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
				
				if (!m_CurrentLightData)
					continue;
				
				vector mat[4];
				owner.GetWorldTransform(mat);
				
				vector pos = m_CurrentLightData.GetLightOffset().Multiply4(mat);
				
				if(m_CurrentLightData.m_rLightToChangeOnXOBMaterial && m_CurrentLightData.m_rLightToChangeOffXOBMaterial && m_CurrentLightData.m_rLightToChangeXOB)
					GetGame().GetWorld().QueryEntitiesBySphere(pos, 1, ProcessEntity);
			};

			RemoveLights();
			
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