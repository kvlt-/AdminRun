#include "stdafx.h"
#include "AppDB.h"
#include "Shortcut.h"
#include "AppListCtrl.h"

#define ICON_SIZE           32
#define ITEM_MARGIN         5
#define ITEM_HEIGHT         (ICON_SIZE + 2 * ITEM_MARGIN)
#define ITEM_TEXT_MARGIN    20

CAppListCtrl::CAppListCtrl() : CListBox()
{
}


CAppListCtrl::~CAppListCtrl()
{
}


BEGIN_MESSAGE_MAP(CAppListCtrl, CListBox)
END_MESSAGE_MAP()

void CAppListCtrl::MeasureItem(LPMEASUREITEMSTRUCT pMS)
{
    pMS->itemHeight = ITEM_HEIGHT;
}

int CAppListCtrl::CompareItem(LPCOMPAREITEMSTRUCT pCS)
{
    CShortcut *pItem1 = (CShortcut *)pCS->itemData1;
    CShortcut *pItem2 = (CShortcut *)pCS->itemData2;

    if (!pItem1 || !pItem2) return 0;

    return _tcsicmp(pItem1->GetFilePath(), pItem2->GetFilePath());
}

void CAppListCtrl::DrawItem(LPDRAWITEMSTRUCT pDS)
{
    CShortcut *pItem = (CShortcut *)pDS->itemData;
    
    if (!pItem) return;

    CRect rc(pDS->rcItem);

    CDC * pdc = CDC::FromHandle(pDS->hDC);
    CDC dcMem;
    dcMem.CreateCompatibleDC(pdc);

    CBitmap bmp;
    bmp.CreateCompatibleBitmap(pdc, rc.Width(), rc.Height());

    CBitmap * pOldBmp = dcMem.SelectObject(&bmp);
    {
        CRect rcItem(rc);
        rcItem.MoveToXY(0, 0);

        COLORREF rgbBack = (pDS->itemState & ODS_SELECTED) ? RGB(240, 240, 240) : RGB(255, 255, 255);
        dcMem.FillRect(rcItem, &CBrush(rgbBack));

        ::DrawIconEx(dcMem.m_hDC, ITEM_MARGIN, ITEM_MARGIN, pItem->GetIcon(), ICON_SIZE, ICON_SIZE, 0, NULL, DI_NORMAL);

        CRect rcText;
        rcText.left = ITEM_MARGIN + ICON_SIZE + ITEM_TEXT_MARGIN;
        rcText.top = ITEM_MARGIN;
        rcText.right = rcItem.right;
        rcText.bottom = rcItem.bottom - ITEM_MARGIN;

        CFont * pOldFont = dcMem.SelectObject(this->GetFont());
        dcMem.SetBkMode(TRANSPARENT);

        dcMem.DrawText(pItem->GetName(), rcText, DT_SINGLELINE | DT_LEFT | DT_VCENTER);

        dcMem.SelectObject(pOldFont);

        pdc->BitBlt(rc.left, rc.top, rc.Width(), rc.Height(), &dcMem, 0, 0, SRCCOPY);
    }
    dcMem.SelectObject(pOldBmp);
}

