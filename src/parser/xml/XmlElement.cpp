//
// Created by anarion on 4/6/20.
//

#include <parser/MapParser.h>
#include "parser/xml/XmlElement.h"

anarion::XmlElement *parseXml(const char *expression, anarion::size_type length);

anarion::XmlElement *anarion::XmlElement::parse(const char *expression, anarion::size_type length) {
    return parseXml(expression, length);
}

anarion::XmlElement::~XmlElement() {
    for (auto it = childs.begin_iterator(); it != childs.end_iterator(); ++it) {
        delete *it;
    }
}

