#pragma once
#include "SlateCore/Widgets/SWindow.h"
class SSplitter : public SWindow
{
    SWindow* SideLT; // Left or Top
    SWindow* SideRB; // Right or Bottom
};

class SSplitterH : public SSplitter
{
};

class SSplitterV : public SSplitter
{
};