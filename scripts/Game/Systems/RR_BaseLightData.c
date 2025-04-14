[BaseContainerProps()]
class RR_BaseLightData : SCR_BaseLightData
{	
	[Attribute(uiwidget: "resourcePickerThumbnail", params: "xob")]
	ResourceName m_rLightToChangeXOB;
	
	[Attribute(uiwidget: "resourcePickerThumbnail", params: "emat")]
	ResourceName m_rLightToChangeOnXOBMaterial;
	
	[Attribute(uiwidget: "resourcePickerThumbnail", params: "emat")]
	ResourceName m_rLightToChangeOffXOBMaterial;
	
	[Attribute()]
	int m_iLightToChangeXOBMaterialNumber;
}