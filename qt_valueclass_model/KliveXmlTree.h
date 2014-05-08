#ifndef KliveXmlTree_H
#define KliveXmlTree_H

#include <QObject>
#include <QList>
#include <QDomDocument>
#include <QDomElement>

class QIODevice;
class PipelineConfiguration;

class KliveXmlTree : public QObject
{
    Q_OBJECT

public:
    KliveXmlTree(QObject* parent = 0);

    bool read(QIODevice* device);
    bool write(QIODevice* device);

private:
    /* Parse "klive/streams/stream" tag from KLive XML document. Eg:
     * <streams>
     *   <stream>
     *     <uid>1</uid>
     *     <name>Stream 1</name>
     *     <description>Sample Stream</description>
     *     <nodes>
     *       ...
     *     </nodes>
     *   </stream>
     * </streams>
     */
    void parse_stream(const QDomElement& stream);

    /* Parse "klive/streams/stream/nodes/node" tag from KLive XML document. Eg:
     * <nodes>
     *   <node>
     *     <uid>1</uid>
     *     <type>decodebin</type>
     *     <properties>
     *       <property name="device-number" value="0" />
     *       <property name="connection" value="3" />
     *     </properties>
     *     <previousNodes>
     *       <node uid="1" srcpad="src" dstpad="sink" type="static" mime="dummy" />
     *     </previousNodes>
     *   </node>
     * </nodes>
     */
    void parse_stream_node(const QDomElement& node, PipelineConfiguration* ptConfiguration);

private:
    QDomDocument m_domDocument;

    /* There is 1 PipelineConfiguration for each "stream" item parsed from XML. */
    QList<PipelineConfiguration*> m_configurations;
};

#endif // KliveXmlTree_H
