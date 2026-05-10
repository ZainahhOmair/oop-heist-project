#ifndef INTERACTABLE_H //this should make the file name
#define INTERACTABLE_H

class Interactable {  //class name should match file name
public:
    virtual void interact() = 0;
    virtual ~Interactable() {}
};

#endif