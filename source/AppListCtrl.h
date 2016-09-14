#pragma once

class CAppListCtrl : public CListBox
{
public:
    CAppListCtrl();
    ~CAppListCtrl();

protected:
    DECLARE_MESSAGE_MAP()
public:
    void MeasureItem(LPMEASUREITEMSTRUCT pMS);
    int CompareItem(LPCOMPAREITEMSTRUCT pCS);
    void DrawItem(LPDRAWITEMSTRUCT pDS);

    void DrawEntire(LPDRAWITEMSTRUCT lpDStruct);
    void DrawSelected(LPDRAWITEMSTRUCT lpDStruct);
    void DrawBitmaps(HDC hDC, RECT rect, int checked);
};

