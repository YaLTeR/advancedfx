#include <hlae/lists/settingspage.h>

hlaeListElementSettingsPage::hlaeListElementSettingsPage(
	hlaeSettingsPageTemplate* page, const wxString& name, bool is_advanced)
{
	m_pagelist = new hlaeListSettingsPage;
	m_pagelist->DeleteContents(true);

	m_page = page;
	m_name = name;
	m_isadvanced = is_advanced;
}

hlaeListElementSettingsPage::~hlaeListElementSettingsPage()
{
	m_pagelist->Clear();
	delete m_pagelist;
}

bool hlaeListElementSettingsPage::IsAdvanced()
{
	return m_isadvanced;
}

const wxString& hlaeListElementSettingsPage::GetName()
{
	return m_name;
}

hlaeSettingsPageTemplate* hlaeListElementSettingsPage::GetPage()
{
	return m_page;
}

hlaeListElementSettingsPage* hlaeListElementSettingsPage::GetElement(int index)
{
	return (m_pagelist->Item(index))->GetData();
}

size_t hlaeListElementSettingsPage::GetCount()
{
	return m_pagelist->GetCount();
}

void hlaeListElementSettingsPage::Append(hlaeListElementSettingsPage* page_element)
{
	m_pagelist->Append(page_element);
}