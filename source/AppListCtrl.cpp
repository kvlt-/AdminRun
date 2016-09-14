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

    DrawIconEx(pDS->hDC, pDS->rcItem.left + ITEM_MARGIN, pDS->rcItem.top + ITEM_MARGIN, pItem->GetIcon(), ICON_SIZE, ICON_SIZE, 0, NULL, DI_NORMAL);

    CRect rect(pDS->rcItem);
    rect.top    += ITEM_MARGIN;
    rect.bottom -= ITEM_MARGIN;
    rect.left   += ITEM_MARGIN + ICON_SIZE + ITEM_TEXT_MARGIN;
    rect.right  -= ITEM_MARGIN;

    CString csPath = pItem->GetFilePath();
    CString csLabel;
    int iSlash  = csPath.ReverseFind(_T('\\'));
    int iDot    = csPath.ReverseFind(_T('.'));
    csLabel     = csPath.Mid(iSlash + 1, iDot - iSlash - 1);

    DrawText(pDS->hDC, csLabel, csLabel.GetLength(), &rect, DT_SINGLELINE | DT_LEFT | DT_VCENTER);
}

