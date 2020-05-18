var cmnObj = objects['commonObj'];
var cfObj = objects['codeFlowObj'];

//will not really be needed, cannot perform anything on code flow objects

var trackObjects = [];

$(function() {
	populateObjects();
	$('#track_modal').on('hidden.bs.modal', function() {
		updateDisplay();
	});
});

function clearObjectPane() {
	document.getElementById('objects_container').innerHTML = '';
}

function populateObjects() {
	var htmlProduced = '';
	var containerHead = '<div class="container-fluid" id="trackContainer-';
	var containerTail = '">';
	var checkBoxHead = '<input type="checkbox" value="" id="cBoxId-';
	var checkBoxTail = '" onChange="objTrackStatusChanged(this.id);"/>';
	var labelHead = '<label class="ml-2 clickable-content dialog-text" id="objTrackLbl-';
	var labelTail = '" for="cBoxId-';

	var comObj = objects['commonObj'];
	var numObjs = Object.keys(comObj).length;
	var i;

	for (i = 1; i <= numObjs; i++) {
		htmlProduced += containerHead + i + containerTail;
		htmlProduced += checkBoxHead + i + checkBoxTail;
		htmlProduced += labelHead + i + labelTail + i + '">';
		htmlProduced += comObj[i]['name'];
		htmlProduced += '</label>';
		htmlProduced += '</div>';
	}

	document.getElementById('modal_body').innerHTML = htmlProduced;
}
function setCheckBoxForId(labelId) {
	var cbId = 'cBoxId-' + labelId.split('-')[1];

	document.getElementById(cbId).checked = !document.getElementById(cbId).checked;
	objTrackStatusChanged(cbId);
}
function objTrackStatusChanged(id) {
	id = id.split('-')[1];
	var ret = trackObjects.indexOf(id);

	if (ret > -1)
		//remove
		trackObjects.splice(ret, 1);
	else
		//add
		trackObjects.push(id);
}

var wantDisplay = [];
var fakeBottomHr =
	'<hr class="mt-2 mb-3" style="visibility: hidden"/><hr class="mt-2 mb-3" style="visibility: hidden"/><hr class="mt-2 mb-3" style="visibility: hidden"/>';
var hr = '<hr class="mt-2 mb-3"/>';
var lhsLabel = '<label class="d-block object-lhs ml-2">';
var rhsLabel = '<label class="object-rhs-special ml-2">';
var tailLabel = '</label>';
function generateObjContent(objName, objId, objState, toIndex) {
	var htmlProduced = '';
	var currObjValue = getObjCurrValue(objId, objState, toIndex);
	//generate name row
	htmlProduced += lhsLabel;
	htmlProduced += 'Name: ';
	htmlProduced += rhsLabel;
	htmlProduced += objName;
	htmlProduced += tailLabel;
	htmlProduced += tailLabel;

	//generate struct type
	htmlProduced += lhsLabel;
	htmlProduced += 'Structure: ';
	htmlProduced += rhsLabel;
	htmlProduced += currObjValue[0];
	htmlProduced += tailLabel;
	htmlProduced += tailLabel;

	//generate data type
	htmlProduced += lhsLabel;
	htmlProduced += 'Data Type: ';
	htmlProduced += rhsLabel;
	htmlProduced += currObjValue[1];
	htmlProduced += tailLabel;
	htmlProduced += tailLabel;

	//generate current value
	htmlProduced += lhsLabel;
	htmlProduced += 'Current Value: ';
	htmlProduced += rhsLabel;
	htmlProduced += currObjValue[2];
	htmlProduced += tailLabel;
	htmlProduced += tailLabel;
	return htmlProduced;
}
function updateDisplay() {
	if (trackObjects.length == 0) return;
	var htmlProduced = '';
	wantDisplay.forEach((obj) => {
		if (trackObjects.indexOf(String(obj.id)) > -1) {
			htmlProduced += generateObjContent(obj.name, obj.id, obj.state, obj.withIndex);
			htmlProduced += hr;
		}
	});
	htmlProduced += fakeBottomHr;

	document.getElementById('objects_container').innerHTML = htmlProduced;
}

function verifyTrack(objList) {
	wantDisplay = objList;
	updateDisplay();
}

function getObjCurrValue(id, state, index) {
	var currentValue = [];
	cmnObj[id]['modList'].forEach((mod) => {
		//resolve

		if (mod['id'] == state) {
			if (mod['vecMod'] == false) {
				currentValue.push(mod['structType']);
				currentValue.push(mod['dataType']);
				currentValue.push(mod['value']);
				return currentValue;
			}
		}
	});
	return currentValue;
}

function clearWantDisplay() {
	wantDisplay = [];
	clearPanes();
}

function getCommonObjById(id) {
	return cmnObj[id];
}

function getCommonObjNameById(id) {
	return cmnObj[id]['name'];
}

function getCodeFlowObjNameById(id) {
	return cfObj[id]['name'];
}

function getUpperLabelData(line) {
	line = line.split('-')[1];
}
