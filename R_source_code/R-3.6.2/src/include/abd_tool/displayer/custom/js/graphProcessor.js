var canvas;

var options = {
	manipulation: false,
	height: '90%',
	layout: {
		hierarchical: {
			enabled: true,
			levelSeparation: 300
		}
	},
	physics: {
		hierarchicalRepulsion: {
			nodeDistance: 300
		}
	}
};

$(function() {
	//load function
	var container = document.getElementById('exec-flow-graph');
	var data = { nodes: nodes, edges: edges };
	var gph = new vis.Network(container, data, options);
});
var nodes = [
	{
		id: 'cfg_0x00405a2e',
		size: 150,
		label:
			'0x00405a2e:\nmov    DWORD PTR ss:[esp + 0x000000b0], 0x00000002\nmov    DWORD PTR ss:[ebp + 0x00], esi\ntest   bl, 0x02\nje     0x00405a49<<Insn>>\n',
		color: '#FFFFFFFF',
		shape: 'box',
		font: { face: 'monospace', align: 'left' }
	},
	{
		id: 'cfg_0x00405a49',
		size: 150,
		label: '0x00405a49:\ntest   bl, 0x01\nje     0x00405a62<<Insn>>\n',
		color: '#FFFFFFFF',
		shape: 'box',
		font: { face: 'monospace', align: 'left' }
	},
	{
		id: 'cfg_0x00405a55',
		size: 150,
		label: '0x00405a55:\nmov    ecx, DWORD PTR ss:[esp + 0x1c]\npush   ecx\ncall   0x004095c6<<Func>>\n',
		color: '#FFFFFFFF',
		shape: 'box',
		font: { face: 'monospace', align: 'left' }
	},
	{
		id: 'cfg_0x00405a62',
		size: 150,
		label:
			'0x00405a62:\nmov    eax, 0x00000002\nmov    ecx, DWORD PTR ss:[esp + 0x000000a8]\nmov    DWORD PTR fs:[0x00000000], ecx\npop    ecx\npop    esi\npop    ebp\npop    ebx\nadd    esp, 0x000000a4\nret\n',
		color: '#FFFFFFFF',
		shape: 'box',
		font: { face: 'monospace', align: 'left' }
	},
	{
		id: 'cfg_0x004095c6',
		size: 150,
		label: '0x004095c6:\nmov    edi, edi\npush   ebp\nmov    ebp, esp\npop    ebp\njmp    0x00417563<<Func>>\n',
		color: '#FFFFFFFF',
		shape: 'box',
		font: { face: 'monospace', align: 'left' }
	},
	{
		id: 'cfg_0x00405a39',
		size: 150,
		label:
			'0x00405a39:\nand    ebx, 0xfd<-0x03>\nlea    ecx, [esp + 0x34]\nmov    DWORD PTR ss:[esp + 0x10], ebx\ncall   0x00403450<<Func>>\n',
		color: '#FFFFFFFF',
		shape: 'box',
		font: { face: 'monospace', align: 'left' }
	},
	{
		id: 'cfg_0x00403450',
		size: 150,
		label:
			'0x00403450:\npush   0xff<-0x01>\npush   0x0042fa64\nmov    eax, DWORD PTR fs:[0x00000000]\npush   eax\npush   ecx\npush   ebx\npush   ebp\npush   esi\npush   edi\nmov    eax, DWORD PTR ds:[0x0043dff0<.data+0x0ff0>]\nxor    eax, esp\npush   eax\nlea    eax, [esp + 0x18]\nmov    DWORD PTR fs:[0x00000000], eax\nmov    esi, ecx\nmov    DWORD PTR ss:[esp + 0x14], esi\npush   esi\nmov    DWORD PTR ss:[esp + 0x24], 0x00000004\ncall   0x0042f03f<<Func>>\n',
		color: '#FFFFFFFF',
		shape: 'box',
		font: { face: 'monospace', align: 'left' }
	},
	{
		id: 'cfg_0x00405a4e',
		size: 150,
		label: '0x00405a4e:\ncmp    DWORD PTR ss:[esp + 0x30], 0x10\njb     0x00405a62<<Insn>>\n',
		color: '#FFFFFFFF',
		shape: 'box',
		font: { face: 'monospace', align: 'left' }
	},
	{
		id: 'cfg_0x00405a5f',
		size: 150,
		label: '0x00405a5f:\nadd    esp, 0x04\n',
		color: '#FFFFFFFF',
		shape: 'box',
		font: { face: 'monospace', align: 'left' }
	}
];
function appendToNodes(id, label) {
	let node = {
		id: '',
		size: 150,
		label: '',
		color: '#FFFFFFFF',
		shape: 'box',
		font: { face: 'monospace', align: 'left' }
	};

	node.id = id;
	let fullLabel = '';
	label.forEach((str) => {
		fullLabel += str + '\n';
	});
	node.label = fullLabel;
	nodes.push(node);
}
var edges = [
	{ from: 'cfg_0x00405a2e', to: 'cfg_0x00405a39', arrows: 'to', physics: false, smooth: { type: 'cubicBezier' } },
	{ from: 'cfg_0x00405a2e', to: 'cfg_0x00405a49', arrows: 'to', physics: false, smooth: { type: 'cubicBezier' } },
	{ from: 'cfg_0x00405a49', to: 'cfg_0x00405a4e', arrows: 'to', physics: false, smooth: { type: 'cubicBezier' } },
	{ from: 'cfg_0x00405a49', to: 'cfg_0x00405a62', arrows: 'to', physics: false, smooth: { type: 'cubicBezier' } },
	{ from: 'cfg_0x00405a55', to: 'cfg_0x00405a5f', arrows: 'to', physics: false, smooth: { type: 'cubicBezier' } },
	{ from: 'cfg_0x00405a55', to: 'cfg_0x004095c6', arrows: 'to', physics: false, smooth: { type: 'cubicBezier' } },
	{ from: 'cfg_0x004095c6', to: 'cfg_0x00417563', arrows: 'to', physics: false, smooth: { type: 'cubicBezier' } },
	{ from: 'cfg_0x00405a39', to: 'cfg_0x00403450', arrows: 'to', physics: false, smooth: { type: 'cubicBezier' } },
	{ from: 'cfg_0x00405a39', to: 'cfg_0x00405a49', arrows: 'to', physics: false, smooth: { type: 'cubicBezier' } },
	{ from: 'cfg_0x00403450', to: 'cfg_0x00403489', arrows: 'to', physics: false, smooth: { type: 'cubicBezier' } },
	{ from: 'cfg_0x00403450', to: 'cfg_0x0042f03f', arrows: 'to', physics: false, smooth: { type: 'cubicBezier' } },
	{ from: 'cfg_0x00405a4e', to: 'cfg_0x00405a55', arrows: 'to', physics: false, smooth: { type: 'cubicBezier' } },
	{ from: 'cfg_0x00405a4e', to: 'cfg_0x00405a62', arrows: 'to', physics: false, smooth: { type: 'cubicBezier' } },
	{ from: 'cfg_0x00405a5f', to: 'cfg_0x00405a62', arrows: 'to', physics: false, smooth: { type: 'cubicBezier' } }
];
function addEdge(fromId, toId) {
	let edge = {
		from: '',
		to: '',
		arrows: 'to',
		physics: false,
		smooth: { type: 'cubicBezier' }
	};
	edge.from = fromId;
	edge.to = toId;
	edges.push(edge);
}
