var cmnObj = objects['commonObj'];
var cfObj = objects['codeFlowObj'];

$(function() {
	//populateObjects();

	concatObjIds();
	$('#track_modal').on('hidden.bs.modal', function() {
		updateDisplay();
	});
});

function clearObjectPane() {
	document.getElementById('objects_container').innerHTML = '';
}

var trackObjects = [];
var objIdsByName = [];

function objNameSeen(name) {
	var i;
	for (i = 0; i < objIdsByName.length; i++) {
		if (objIdsByName[i].name == name) return i;
	}
	return -1;
}
function concatObjIds() {
	var comObj = objects['commonObj'];
	var numObjs = Object.keys(comObj).length;
	var i;
	for (i = 1; i < numObjs; i++) {
		var obj = {
			name: '',
			ids: []
		};
		obj.name = cmnObj[i]['name'];
		var found = objNameSeen(obj.name);
		if (found == -1) {
			//not found
			obj.ids.push(String(i));
			objIdsByName.push(obj);
		} else {
			//found == id
			objIdsByName[found].ids.push(String(i));
		}
	}

	populateObjects();
}
function objIdsToString(index) {
	var ret = '';
	var first = 1;
	objIdsByName[index].ids.forEach((element) => {
		if (first) {
			first = 0;
			ret += element;
		} else {
			ret += '-' + element;
		}
	});

	return ret;
}
function populateObjects() {
	//html generics
	var htmlProduced = '';
	var containerHead = '<div class="container-fluid">';
	var cBoxHead = '<input type="checkbox" value="" id="cBoxId-';
	var cBoxTail = ' onChange="objTrackStatusChanged(this.name, this.id);"/>';
	var cBoxName = '" name="';
	var lblHead = '<label class="ml-2 clickable-content dialog-text" id="objTrackLbl-';
	var lblTail = '" for="cBoxId-';
	var index = -1;
	objIdsByName.forEach((obj) => {
		var ids = objIdsToString(++index);
		htmlProduced += containerHead;
		htmlProduced += cBoxHead + ids + cBoxName + obj.name + '"' + cBoxTail;
		htmlProduced += lblHead + ids + lblTail + ids + '">';
		htmlProduced += obj.name;
		htmlProduced += '</label>';
		htmlProduced += '</div>';
	});

	document.getElementById('modal_body').innerHTML = htmlProduced;
}

function objTrackStatusChanged(name, ids) {
	ids = ids.split('-');

	for (i = 1; i < ids.length; i++) {
		var ret = trackObjects.indexOf(ids[i]);

		if (ret > -1)
			//remove
			trackObjects.splice(ret, 1);
		else {
			//add

			trackObjects.push(ids[i]);
		}
	}
	console.log(trackObjects);
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
		console.log('update display...');
		console.log('WantDisplay obj.id: ' + obj.id);
		console.log('track: ' + trackObjects);
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
