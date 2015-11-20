import xml.etree.ElementTree as ElementTree
tree = ElementTree.parse('scene01.xml')
root = tree.getroot()

def print_node(node):
	print node.tag, node.attrib
	for child in root:
		print child.tag, child.attrib

print_node(root)