#include "KliveXmlTree.h"

#include "configuration/pipelineconfiguration.h"
#include "configuration/configurationnode.h"
#include "configuration/property.h"
#include "configuration/previousnoderef.h"

#include <QtDebug>
#include <QString>

#include <QIODevice>

using namespace GStreamer1_0;

KliveXmlTree::KliveXmlTree(QObject* parent)
    : QObject(parent)
{}

bool KliveXmlTree::read(QIODevice* device)
{
    QString errorMsg;
    int errorLine;
    int errorColumn;
    if (!m_domDocument.setContent(device, true, &errorMsg, &errorLine, &errorColumn)) {
        qDebug() << "[KliveXmlTree::read]"
                 << QString("Parse error at line %1, column %2:\n%3")
                    .arg(errorLine)
                    .arg(errorColumn)
                    .arg(errorStr);
        return false;
    }

    QDomElement root = m_domDocument.documentElement();
    if (root.tagName() != "klive") {
        qDebug() << "[KliveXmlTree::read]"
                 << "Input is not a KLIVE file";
        return false;
    }
    else if (root.hasAttribute("version")
             && root.attribute("version") != "1.0") {
        qDebug() << "[KliveXmlTree::read]"
                 << "Input is not a KLIVE file version 1.0";
        return false;
    }

    QDomElement name = root.firstChildElement("name");
    QDomElement description = root.firstChildElement("description");
    QDomElement creationDate = root.firstChildElement("creationDate");
    QDomElement updateDate = root.firstChildElement("updateDate");

    QDomElement streams = root.firstChildElement("streams");
    if (!streams.isNull()) {
        QDomElement stream = streams.firstChildElement("stream");
        while (!stream.isNull()) {
            parse_stream(stream);
            stream = stream.nextSiblingElement("stream");
        }
    }

    return true;
}

bool KliveXmlTree::write(QIODevice* device)
{

}

void KliveXmlTree::parse_stream(const QDomElement& stream)
{
    QDomElement uid = stream.firstChildElement("uid");
    QDomElement name = stream.firstChildElement("name");
    QDomElement description = stream.firstChildElement("description");

    QDomElement nodes = stream.firstChildElement("nodes");
    if (!nodes.isNull()) {
        PipelineConfiguration* ptConfiguration = new PipelineConfiguration();
        QDomElement node = nodes.firstChildElement("node");
        while (!node.isNull()) {
            parse_stream_node(node, ptConfiguration);
            node = node.nextSiblingElement("node");
        }
        ptConfiguration->refactorNodeRefs();
        m_configurations.push_back(ptConfiguration);
    }
}

void KliveXmlTree::parse_stream_node(const QDomElement& node, PipelineConfiguration* ptConfiguration)
{
    ConfigurationNode* ptNode = new ConfigurationNode();

    QDomElement uid = node.firstChildElement("uid");
    if (!uid.isNull()) {
        ptNode->insertUID(uid.text().toStdString());
    }

    QDomElement type = node.firstChildElement("type");
    if (!type.isNull()) {
        ptNode->insertType(type.text().toStdString());
    }

    QDomElement properties = node.firstChildElement("properties");
    if (!properties.isNull()) {
        QDomElement property = properties.firstChildElement("property");
        while (!property.isNull()) {
            Property* ptProperty = new Property();
            if (property.hasAttribute("name")) {
                ptProperty->setName(property.attribute("name"));
            }
            if (property.hasAttribute("value")) {
                ptProperty->setValue(property.attribute("value"));
            }
            ptNode->insertProperty(ptProperty);
            property = property.nextSiblingElement("property");
        }
    }

    QDomElement previousNodes = node.firstChildElement("previousNodes");
    if (!previousNodes.isNull()) {
        QDomElement previousNode = previousNodes.firstChildElement("node");
        while (!previousNode.isNull()) {
            PreviousNodeRef* ptPreviousNodeRef = new PreviousNodeRef();
            if (previousNode.hasAttribute("uid")) {
                ptPreviousNodeRef->setUID(previousNode.attribute("uid"));
            }
            if (previousNode.hasAttribute("srcpad")) {
                ptProperty->setSrcPad(previousNode.attribute("srcpad"));
            }
            if (previousNode.hasAttribute("dstpad")) {
                ptProperty->setDstPad(previousNode.attribute("dstpad"));
            }
            if (previousNode.hasAttribute("mime")) {
                ptProperty->setMime(previousNode.attribute("mime"));
            }
            if (previousNode.hasAttribute("type")) {
                QString type = previousNode.attribute("type");
                if (type == "dynamic") {
                    ptPreviousNodeRef->setConnectionType(PreviousNodeRef::dynamic);
                } else if (type == "onrequest") {
                    ptPreviousNodeRef->setConnectionType(PreviousNodeRef::onrequest);
                } else if (type == "static") {
                    ptPreviousNodeRef->setConnectionType(PreviousNodeRef::staticc);
                } else if (type == "unique") {
                    ptPreviousNodeRef->setConnectionType(PreviousNodeRef::unique);
                }
            }
            ptNode->insertPreviousNodeRef(ptPreviousNodeRef);
            previousNode = previousNode.nextSiblingElement("node");
        }
    }

    ptConfiguration->insertNode(ptNode);
}
