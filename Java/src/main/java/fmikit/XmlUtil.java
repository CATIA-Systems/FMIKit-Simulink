package fmikit;

import java.util.*;
import org.w3c.dom.*;

public final class XmlUtil {

    private XmlUtil() {
    }

    public static Element getChildElement(Element element, String tagName) {

        NodeList elements = element.getElementsByTagName(tagName);

        for (int i = 0; i < elements.getLength(); i++) {
            Node node = elements.item(i);
            if (node instanceof Element) {
                return (Element) node;
            }
        }

        return null;
    }

    public static List<Element> elementsByTagName(Element element, String tagName) {

        NodeList n = element.getElementsByTagName(tagName);

        return n.getLength() == 0 ? Collections.<Element>emptyList() : new ElementListWrapper(n);
    }

    public static List<Element> asElementList(NodeList n) {

        List<Element> elements = new ArrayList<Element>();

        for (int i = 0; i < n.getLength(); i++) {
            Node node = n.item(i);
            if (node instanceof Element) {
                elements.add((Element) node);
            }
        }

        return elements;
    }

    public static List<Node> asList(NodeList n) {
        return n.getLength() == 0 ?
                Collections.<Node>emptyList() : new NodeListWrapper(n);
    }

    static final class ElementListWrapper extends AbstractList<Element>
            implements RandomAccess {

        private final NodeList list;

        ElementListWrapper(NodeList l) {
            l.getLength();
            list = l;
        }

        public Element get(int index) {
            return (Element)list.item(index);
        }

        public int size() {
            return list.getLength();
        }
    }

    static final class NodeListWrapper extends AbstractList<Node>
            implements RandomAccess {

        private final NodeList list;

        NodeListWrapper(NodeList l) {
            list = l;
        }

        public Node get(int index) {
            return list.item(index);
        }

        public int size() {
            return list.getLength();
        }
    }
}