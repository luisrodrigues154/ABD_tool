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
	for (i = 1; i <= numObjs; i++) {
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
}

var wantDisplay = [];
var fakeBottomHr =
	'<hr class="mt-2 mb-3" style="visibility: hidden"/><hr class="mt-2 mb-3" style="visibility: hidden"/><hr class="mt-2 mb-3" style="visibility: hidden"/>';
var hr = '<hr class="mt-2 mb-3"/>';

function genObjCol(colName, colContent) {
	var htmlProduced = '';

	htmlProduced += '<div class="col col-md-auto col1">';
	htmlProduced += '<label class="event-content">' + colName + '</label>';
	console.log('size: ' + colContent.length);
	colContent.forEach((element) => {
		console.log('element: ' + element);
		htmlProduced += '<label class="event-rhs d-block ">' + element + '</label>';
	});

	htmlProduced += '</div>';

	return htmlProduced;
}

function generateObjRow(objName, objContent) {
	var htmlProduced = '';
	htmlProduced +=
		'<label class="event-title" id="obj_title">Name: <label class="object-rhs-special">' +
		objName +
		'</label></label>';
	htmlProduced += '<div class="container-fluid" id="obj_content">';
	htmlProduced += '<div class="row">';
	htmlProduced += objContent;
	htmlProduced += '</div>';
	htmlProduced += '</div>';
	return htmlProduced;
}
function getDisplayForBigVectors(vector) {
	var counter = 1;

	var strList = [];
	var strAux = '';
	var len = vector.length;
	var i;
	var space = '&nbsp';
	strAux += '[' + space + space;
	for (i = 0; i < len; i++, counter++) {
		strAux += vector[i] + (i + 1 < len ? ',' : '');
		strAux += space;
		if (counter == 4) {
			strList.push(strAux);
			counter = 0;
			strAux = '';
			strAux += space + space;
		}
	}
	for (i = counter; i <= 4; i++) strAux += space;
	strAux += space + ']';
	strList.push(strAux);

	return strList;
}
function generateObjContent(objName, objId, objState, toIndex) {
	var currObjValue = getObjCurrValue(objId, objState, toIndex);
	var colContent = [];
	var objContent = '';

	//generate name row
	// colContent.push(objName);
	// objContent += genObjCol('Name', colContent);
	// colContent = [];

	//generate struct type
	colContent.push(currObjValue[0]);
	objContent += genObjCol('Structure', colContent);
	colContent = [];

	//generate data type
	colContent.push(currObjValue[1]);
	objContent += genObjCol('Data type', colContent);
	colContent = [];

	//generate data type
	colContent.push(currObjValue[2]);
	objContent += genObjCol('Size', colContent);
	colContent = [];

	//generate current value
	colContent = getDisplayForBigVectors(currObjValue[3]);
	objContent += genObjCol('Current value', colContent);
	colContent = [];

	return generateObjRow(objName, objContent);
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
				currentValue.push(mod['nElements']);
				currentValue.push(mod['vector']);
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
