#pragma once

class CAppListCtrl : public CListBox
{
public:
    CAppListCtrl();
    ~CAppListCtrl();

protected:
    DECLARE_MESSAGE_MAP()

public:
    void MeasureItem(LPMEASUREITEMSTRUCT pMS) override;
    int CompareItem(LPCOMPAREITEMSTRUCT pCS) override;
    void DrawItem(LPDRAWITEMSTRUCT pDS) override;

    //void DrawEntire(LPDRAWITEMSTRUCT lpDStruct);
    //void DrawSelected(LPDRAWITEMSTRUCT lpDStruct);
    //void DrawBitmaps(HDC hDC, RECT rect, int checked);
};

