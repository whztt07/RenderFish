import xml.etree.ElementTree as ElementTree

xml_node_list = []

def print_node(node, depth):
	print '  '*depth, node.tag, node.attrib
	for child in node:
		#print child.tag, child.attrib
		print_node(child, depth + 1)

def parse_xml(xml_file_path):
	tree = ElementTree.parse(xml_file_path)
	root = tree.getroot()
	print_node(root, 0)

if __name__ == '__main__':
	parse_xml("scene01.xml")