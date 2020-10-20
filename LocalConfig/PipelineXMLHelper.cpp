#include "PipelineXMLHelper.h"


PipelineXMLHelper::PipelineXMLHelper()
{
}


PipelineXMLHelper::~PipelineXMLHelper()
{
}

int PipelineXMLHelper::Open(const std::string &xmlfile)
{
	int res = 0;

	m_xmlFile = xmlfile;

	if((res = m_doc.LoadFile(xmlfile.c_str())) == tinyxml2::XML_SUCCESS)
	{
		m_root = m_doc.FirstChildElement("Pipeline");
		if(m_root == NULL)
		{
			res = -2;
		}
	}

	return res;
}

int PipelineXMLHelper::WriteSettingsToXml(std::list<BaseSettingPtr> *confs)
{
	if(confs == NULL)
	{
		return -1;
	}

	if(!confs->empty())
	{
		m_pSettingList = confs;

		ReadBin(m_root, false);
	}

	return 0;
}

int PipelineXMLHelper::ReadSettingsFromXml(std::list<BaseSettingPtr> *confs)
{
	m_pSettingList = confs;

	ReadBin(m_root);

	return 0;
}

int PipelineXMLHelper::SaveToFile(const std::string &xmlfile)
{
	return m_doc.SaveFile(xmlfile.c_str());
}

void PipelineXMLHelper::Close()
{
	m_doc.Clear();
}


BaseSettingPtr PipelineXMLHelper::FindSettingByOwner(const std::string &owner)
{
	if(m_pSettingList != NULL)
	{
		std::list<BaseSettingPtr>::const_iterator ite = m_pSettingList->cbegin();
		for(; ite != m_pSettingList->cend(); ite++)
		{
			BaseSettingPtr setting = *ite;
			if(setting->mType > CONF_SETTING_LOG)
			{
				if(((MediaSetting*)setting.get())->mOwnerEle == owner)
				{
					return setting;
				}
			}
		}
	}

	return NULL;
}

void PipelineXMLHelper::WriteElementParam(tinyxml2::XMLElement *ele, BaseSettingPtr &setting)
{
	tinyxml2::XMLElement *subEle = NULL;

	if(setting->mType == CONF_SETTING_NONE)
		return;

	switch(setting->mType)
	{
	case CONF_SETTING_SRC:
		{
			SrcSetting *srcSet = static_cast<SrcSetting*>(setting.get());
			if((subEle = ele->FirstChildElement("FilePath")) != NULL)
				subEle->SetAttribute("value", srcSet->mURI.c_str());

			if((subEle = ele->FirstChildElement("TrunkSize")) != NULL)
				subEle->SetAttribute("value", srcSet->mTrunkSize);

			if((subEle = ele->FirstChildElement("VideoWidth")) != NULL)
				subEle->SetAttribute("value", srcSet->mWidth);

			if((subEle = ele->FirstChildElement("VideoHeight")) != NULL)
				subEle->SetAttribute("value", srcSet->mHeight);

			if((subEle = ele->FirstChildElement("ImgFormat")) != NULL)
				subEle->SetAttribute("value", srcSet->mFormat);

			if((subEle = ele->FirstChildElement("VideoFPS")) != NULL)
				subEle->SetAttribute("value", srcSet->mFps);

			if((subEle = ele->FirstChildElement("SampleRate")) != NULL)
				subEle->SetAttribute("value", srcSet->mSampleRate);

			if((subEle = ele->FirstChildElement("SampleFormat")) != NULL)
				subEle->SetAttribute("value", srcSet->mFormat);

			if((subEle = ele->FirstChildElement("AudioChannel")) != NULL)
				subEle->SetAttribute("value", srcSet->mChannel);
		}
		break;
	case CONF_SETTING_FILTER:
		{
			FilterSetting *filterSet = static_cast<FilterSetting*>(setting.get());

			if((subEle = ele->FirstChildElement("FilterDescription")) != NULL)
				subEle->SetAttribute("value", filterSet->mFilterDesc.c_str());

			if((subEle = ele->FirstChildElement("VideoWidth")) != NULL)
				subEle->SetAttribute("value", filterSet->mWidth);

			if((subEle = ele->FirstChildElement("VideoHeight")) != NULL)
				subEle->SetAttribute("value", filterSet->mHeight);

			if((subEle = ele->FirstChildElement("ImgFormat")) != NULL)
				subEle->SetAttribute("value", filterSet->mFormat);

			if((subEle = ele->FirstChildElement("VideoFPS")) != NULL)
				subEle->SetAttribute("value", filterSet->mFps);

			if((subEle = ele->FirstChildElement("Bitrate")) != NULL)
				subEle->SetAttribute("value", filterSet->mBitRate);

			if((subEle = ele->FirstChildElement("BitrateRange")) != NULL)
			{
				subEle->SetAttribute("min", filterSet->mMinBitRate);
				subEle->SetAttribute("max", filterSet->mMaxBitRate);
			}

			if((subEle = ele->FirstChildElement("QValue")) != NULL)
			{
				subEle->SetAttribute("min", filterSet->mQMin);
				subEle->SetAttribute("max", filterSet->mQMax);
			}

			if((subEle = ele->FirstChildElement("IFrameInterval")) != NULL)
				subEle->SetAttribute("value", filterSet->mISpace);

			if((subEle = ele->FirstChildElement("CodecPreset")) != NULL)
				subEle->SetAttribute("value", filterSet->mPreset.c_str());

			if((subEle = ele->FirstChildElement("CodecControl")) != NULL)
				subEle->SetAttribute("value", filterSet->mCtrl.c_str());

			if((subEle = ele->FirstChildElement("CodecQuality")) != NULL)
				subEle->SetAttribute("value", filterSet->mQuality);

			if((subEle = ele->FirstChildElement("CodecProfile")) != NULL)
				subEle->SetAttribute("value", filterSet->mProfile.c_str());

			if((subEle = ele->FirstChildElement("SampleFormat")) != NULL)
				subEle->SetAttribute("value", filterSet->mFormat);

			if((subEle = ele->FirstChildElement("AudioChannel")) != NULL)
				subEle->SetAttribute("value", filterSet->mChannel);

			if((subEle = ele->FirstChildElement("FrameSampleCount")) != NULL)
				subEle->SetAttribute("value", filterSet->mSampleCount);
		}
		break;
	case CONF_SETTING_SINK:
		{
			SinkSetting *sinkSet = static_cast<SinkSetting*>(setting.get());
			if((subEle = ele->FirstChildElement("InPortCount")) != NULL)
				subEle->SetAttribute("value", sinkSet->mInportCount);

			if((subEle = ele->FirstChildElement("FilePath")) != NULL)
				subEle->SetAttribute("value", sinkSet->mURI.c_str());

			if((subEle = ele->FirstChildElement("VideoWidth")) != NULL)
				subEle->SetAttribute("value", sinkSet->mWidth);

			if((subEle = ele->FirstChildElement("VideoHeight")) != NULL)
				subEle->SetAttribute("value", sinkSet->mHeight);

			if((subEle = ele->FirstChildElement("ImgFormat")) != NULL)
				subEle->SetAttribute("value", sinkSet->mFormat);

			if((subEle = ele->FirstChildElement("SampleRate")) != NULL)
				subEle->SetAttribute("value", sinkSet->mSampleRate);

			if((subEle = ele->FirstChildElement("SampleFormat")) != NULL)
				subEle->SetAttribute("value", sinkSet->mFormat);

			if((subEle = ele->FirstChildElement("AudioChannel")) != NULL)
				subEle->SetAttribute("value", sinkSet->mChannel);
		}
		break;
	default:
		break;
	}

}

BaseSetting* PipelineXMLHelper::ReadElementParam(tinyxml2::XMLElement *ele)
{
	return NULL;
}

void PipelineXMLHelper::ReadBin(tinyxml2::XMLNode *binnode, bool bOnlyRead)
{
	tinyxml2::XMLElement *ele = NULL;

	if(binnode != NULL)
	{
		ele = binnode->ToElement();
		std::string tagName = ele->Name();

		if(tagName != "Pipeline" && tagName != "Bin")
		{
			return;
		}

		tinyxml2::XMLNode *node = binnode->FirstChild();
		while(node != NULL)
		{
			ele = node->ToElement();
			tagName = ele->Name();
			if(tagName == "Bin")
			{
				ReadBin(node, bOnlyRead);
			}
			else
			{
				ReadElement(node, bOnlyRead);
			}

			node = node->NextSibling();
		}

	}
}

void PipelineXMLHelper::ReadElement(tinyxml2::XMLNode *elenode, bool bOnlyRead)
{
	tinyxml2::XMLElement *ele = elenode->ToElement();
	if(ele != NULL)
	{
		tinyxml2::XMLElement *subEle = ele->FirstChildElement();
		while(subEle != NULL)
		{
			if(bOnlyRead)
			{
				BaseSetting* setting = ReadElementParam(ele);
				if(setting != NULL)
				{
					m_pSettingList->push_back(BaseSettingPtr(setting));
				}
			}
			else
			{
				BaseSettingPtr setting = FindSettingByOwner(ele->Attribute("name"));
				WriteElementParam(ele, setting);
			}
			subEle = subEle->NextSiblingElement();
		}	
	}
}
