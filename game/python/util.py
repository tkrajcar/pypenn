# utility functions
# Copyright 2009 Tim Krajcar <allegro@conmolto.org

from ll import ansistyle

def titlebar(text=""):
    if text == "":
        return ansistyle.Text(010,">","-" * 77,"<").string()
    return ansistyle.Text(010,">--",0110,"[",0170,text,0110,"]",010,"---","-" * (70-len(text)),"<").string()

def footerbar(text=""):
    if text == "":
        return ansistyle.Text(010,">","-" * 77,"<").string()
    return ansistyle.Text(010,">--","-" * (70-len(text)),0110,"[",070,text,0110,"]",010,"---<").string()

