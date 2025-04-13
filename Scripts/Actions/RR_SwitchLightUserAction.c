class RR_SwitchLightUserAction : SCR_SwitchLightUserAction
{
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		pOwnerEntity = pOwnerEntity.GetRootParent();
		m_LightComp = SCR_BaseInteractiveLightComponent.Cast(pOwnerEntity.FindComponent(SCR_BaseInteractiveLightComponent));
	}
}