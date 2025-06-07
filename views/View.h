#pragma once

struct ViewSize {
    float w;
    float h;
};

struct ViewPositon {
    float x;
    float y;
};

class View {
public:
    View() { }
    virtual ~View() { }
    virtual void Show() = 0;
    void        setViewSize(const ViewSize& size)  { this->Size = size; }
    ViewSize    getViewSize()                      { return this->Size; }
    void        setViewPos(const ViewPositon& pos) { this->Position = pos; }
    ViewPositon getViewPos()                       { return this->Position; }
private:
    ViewSize Size;
    ViewPositon Position;
};
