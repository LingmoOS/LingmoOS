/*
 * liblingmosdk-system's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */

{
	output = function (message) {
		var output = document.getElementById("StateResult");
		output.innerHTML = output.innerHTML + message + "\n";
	},
	getJsonFromInfo = function (info) {
		if (info == "") {
			return "{}";
		}
		var info_list = info.split("&");
		var info_json = "{";
		for (var i = 0; i < info_list.length; i++) {
			var key_value_list = info_list[i].split(":");
			var key = key_value_list[0];
			var value = key_value_list[1];
			if (i == 0) {
				info_json += "\"" + key + "\"" + ":" + "\"" + value + "\"";
			} else {
				info_json += ",\"" + key + "\"" + ":" + "\"" + value + "\"";
			}
		}
		info_json += "}";
		return info_json;
	},
	isEmptyObject = function (obj) {
		for (var key in obj) {
			return false;
		}
		return true;
	},
	coinhopper = {
		coinReturnJobCompleteCB: function () {},
		coinhopperConnectedCB: function () {},
		//munconnected:function(){},
		connected: function (callback) {
			coinhopperConnectedCB = callback;
			var socket = new WebSocket("ws://localhost:12345");
			socket.onopen = function () {
				new QWebChannel(socket, function (channel) {
					coinhopperobject = channel.objects.coinhopper;
					coinhopperobject.coinreturncompleted.connect(function (return_code, job_id, info) {
						coinReturnJobCompleteCB(return_code, info);
					});
					if (typeof (coinhopperConnectedCB) != "undefined") {
						coinhopperConnectedCB();
					}
				});
			};
			//socket.onclose = function(){
			//	callback("onclose");
			//};
			//socket.onerror = function(error){
			//	callback(error);
			//};
		},
		open: function (callback) {
			coinhopperobject.open(function (return_code) {
				callback(return_code.toString());
			});
		},
		close: function (callback) {
			coinhopperobject.close(function (return_code) {
				callback(return_code.toString());
			});
		},
		getState: function (callback) {
			coinhopperobject.getstatus(function (state_info) {
				if (isEmptyObject(state_info)) {
					callback("Exception", "207");
				} else {
					var state_info_str = state_info.toString().split(",");
					callback(state_info_str[0], state_info_str[1]);
				}
			});
		},
		getInfo: function (callback) {
			coinhopperobject.getinfo(function (info) {
				var info_str = info.toString().split(",");
				var handwareInfoJson = getJsonFromInfo(info_str[0]);
				var deviceStateInfoJson = getJsonFromInfo(info_str[1]);
				var staticInfoJson = getJsonFromInfo(info_str[2]);
				callback(handwareInfoJson, deviceStateInfoJson, staticInfoJson);
			});
		},
		setDefaultOptions: function (options) {
			coinhopperobject.setdefaultoptions(JSON.stringify(options), function (return_code) {
				callback(return_code.toString());
			});
		},
		coinReturn: function (options, num, callback) {
			coinhopperobject.coinreturn(JSON.stringify(options), num, function (result) {
				var result_str = result.toString().split(",");
				callback(result_str[0]);
			});
		},
		coinReturnJobCompleted: function (callback) {
			coinReturnJobCompleteCB = callback;
		}
	},
	coindeposit = {
		startReceiveCompletedCB: function () {},
		coindepositConnectedCB: function () {},
		connected: function (callback) {
			coindepositConnectedCB = callback;
			var socket = new WebSocket("ws://localhost:12345");
			socket.onopen = function () {
				new QWebChannel(socket, function (channel) {
					depositobject = channel.objects.coindeposit;
					depositobject.coincomed.connect(function (return_code, job_id, info) {
						startReceiveCompletedCB(return_code, info);
					});
					if (typeof (coindepositConnectedCB) != "undefined") {
						coindepositConnectedCB();
					}
				});
			};
			//socket.onclose = unconnected;
			//socket.onerror = function(error){
			//	output("socket connect error " + error);
			//};
		},
		open: function (callback) {
			depositobject.open(function (return_code) {
				callback(return_code.toString());
			});
		},
		close: function (callback) {
			depositobject.close(function (return_code) {
				callback(return_code.toString());
			});
		},
		getState: function (callback) {
			depositobject.getstatus(function (state_info) {
				if (isEmptyObject(state_info)) {
					callback("Exception", "207");
				} else {
					var state_info_str = state_info.toString().split(",");
					callback(state_info_str[0], state_info_str[1]);
				}
			});
		},
		getInfo: function (callback) {
			depositobject.getinfo(function (info) {
				var info_str = info.toString().split(",");
				var handwareInfoJson = getJsonFromInfo(info_str[0]);
				var deviceStateInfoJson = getJsonFromInfo(info_str[1]);
				var staticInfoJson = getJsonFromInfo(info_str[2]);
				callback(handwareInfoJson, deviceStateInfoJson, staticInfoJson);
			});
		},
		setDefaultOptions: function (options) {
			depositobject.setdefaultoptions(JSON.stringify(options), function (return_code) {
				callback(return_code.toString());
			});
		},
		startReceive: function (options, callback) {
			depositobject.startrecv(JSON.stringify(options), function (result) {
				var result_str = result.toString().split(",");
				callback(result_str[0]);
			});
		},
		stopReceive: function (callback) {
			depositobject.stoprecv(function (return_code) {
				callback(return_code.toString());
			});
		},
		getCashInfo: function (callback) {
			depositobject.getcashinfo(function (result) {
				var result_str = result.toString().split(",");
				callback(result_str[0], result_str[1]);
			});
		},
		pullCash: function (callback) {
			depositobject.pullcash(function (return_code) {
				callback(return_code.toString());
			});
		},
		pushCash: function (callback) {
			depositobject.pushcash(function (return_code) {
				callback(return_code.toString());
			});
		},
		initDevice: function (callback) {
			depositobject.initdevice(function (return_code) {
				callback(return_code.toString());
			});
		},
		startReceiveCompleted: function (callback) {
			startReceiveCompletedCB = callback;
		}
	},
	nfccardreader = {
		getCardInfoCompletedCB: function () {},
		nfcConnectedCB: function () {},
		cardComedCB: function () {},
		cardLeavedCB: function () {},
		connected: function (callback) {
			nfcConnectedCB = callback;
			var socket = new WebSocket("ws://localhost:12345");
			socket.onopen = function () {
				//output("socket opened");
				new QWebChannel(socket, function (channel) {
					nfccardreaderobject = channel.objects.nfcreader;
					nfccardreaderobject.scannfccardcompleted.connect(function (return_code, job_id, info) {
						getCardInfoCompletedCB(return_code, info);
					});
					nfccardreaderobject.cardcomed.connect(function (info) {
						if (typeof (cardComedCB) != "undefined") {
							cardComedCB(info);
						}
					});
					nfccardreaderobject.cardleaved.connect(function (info, period) {
						if (typeof (cardLeavedCB) != "undefined") {
							cardLeavedCB(info, period);
						}
					});
					if (typeof (nfcConnectedCB) != "undefined") {
						nfcConnectedCB();
					}
				});
			};
		},
		open: function (callback) {
			nfccardreaderobject.open(function (return_code) {
				callback(return_code.toString());
			});
		},
		close: function (callback) {
			nfccardreaderobject.close(function (return_code) {
				callback(return_code.toString());
			});
		},
		getState: function (callback) {
			nfccardreaderobject.getstatus(function (state_info) {
				var state_info_str = state_info.toString().split(",");
				callback(state_info_str[0], state_info_str[1]);
			});
		},
		getInfo: function (callback) {
			nfccardreaderobject.getinfo(function (info) {
				var info_str = info.toString().split(",");
				var handwareInfoJson = getJsonFromInfo(info_str[0]);
				var deviceStateInfoJson = getJsonFromInfo(info_str[1]);
				var staticInfoJson = getJsonFromInfo(info_str[2]);
				callback(handwareInfoJson, deviceStateInfoJson, staticInfoJson);
			});
		},
		setDefaultOptions: function (options, callback) {
			nfccardreaderobject.setdefaultoptions(JSON.stringify(options), function (return_code) {
				callback(return_code.toString());
			});
		},
		resetFromException: function (devid, callback) {
			nfccardreaderobject.resetfromexception(devid, function (return_code) {
				callback(return_code.toString());
			})
		},
		readCardInfo: function (options, callback) {
			nfccardreaderobject.scannfccard(JSON.stringify(options), function (result) {
				var result_str = result.toString().split(",");
				callback(result_str[0]);
			});
		},
		readCardInfoCompleted: function (callback) {
			getCardInfoCompletedCB = callback;
			//callback(result,info);
			//nfccardreaderobject.scannfccardcompleted.connect(function(return_code,job_id,info){
			//	callback(return_code,info);	
			//});
		},
		cardComed: function (callback) {
			cardComedCB = callback;
		},
		cardLeaved: function (callback) {
			cardLeavedCB = callback;
		},
		checkCard: function (callback) {
			nfccardreaderobject.cardcheck(function (return_code) {
				callback(return_code.toString());
			});
		},
		resetDevice: function (callback) {
			nfccardreaderobject.resetdevice(function (return_code) {
				callback(return_code.toString());
			});
		}
	},
	iccardreader = {
		insertCardJobCompletedCB: function () {},
		icConnectedCB: function () {},
		connected: function (callback) {
			icConnectedCB = callback;
			var socket = new WebSocket("ws://localhost:12345");
			socket.onopen = function () {
				new QWebChannel(socket, function (channel) {
					icreaderobject = channel.objects.motorizedcardreader;
					icreaderobject.insertcardcompleted.connect(function (return_code, job_id, result) {
						insertCardJobCompletedCB(return_code);
					});
					if (typeof (icConnectedCB) != "undefined") {
						icConnectedCB();
					}
				});
			};
		},
		open: function (callback) {
			icreaderobject.open(function (return_code) {
				callback(return_code.toString());
			});
		},
		close: function (callback) {
			icreaderobject.close(function (return_code) {
				callback(return_code.toString());
			});
		},
		getState: function (callback) {
			icreaderobject.getstatus(function (state_info) {
				var state_info_str = state_info.toString().split(",");
				callback(state_info_str[0], state_info_str[1]);
			});
		},
		getInfo: function (callback) {
			icreaderobject.getinfo(function (info) {
				var info_str = info.toString().split(",");
				var handwareInfoJson = getJsonFromInfo(info_str[0]);
				var deviceStateInfoJson = getJsonFromInfo(info_str[1]);
				var staticInfoJson = getJsonFromInfo(info_str[2]);
				callback(handwareInfoJson, deviceStateInfoJson, staticInfoJson);
			});
		},
		resetfromexception: function (devid,callback) {
			icreaderobject.resetfromexception(devid,function (return_code) {
				callback(return_code.toString());
			});
		},
		setDefaultOptions: function (options,callback) {
			icreaderobject.setdefaultoptions(JSON.stringify(options), function (return_code) {
				callback(return_code.toString());
			});
		},
		insertCard: function (options, callback) {
			icreaderobject.insertcard(JSON.stringify(options), function (result) {
				if(result==null){
					callback(result_str);
				}
				else{
					var result_str = result.toString().split(",");
					callback(result_str[0]);
				}
			});
		},
		readTracks: function (options,track_id, callback) {
			icreaderobject.readtracks(JSON.stringify(options),track_id, function (result) {
				if(result==null){
					callback(result);
				}
				else{
					var result_str = result.toString().split(",");
					callback(result_str[0],result_str[1]);
				}
			});
		},
		samChipio: function (options,sam_slot_id,cmd, callback) {
			icreaderobject.samchipio(JSON.stringify(options),sam_slot_id,cmd,function (result) {
				if(result==null){
					callback(result);
				}
				else{
					var result_str = result.toString().split(",");
					var str_type="";
					for(var i=1;i<result_str.length;i++){
                    			str_type+=result_str[i];
						console.log(str_type);
						str_type+=", ";
					}
				callback(result_str[0],str_type);
				}
			});
		},
		cpuChipio: function (options, cmd_cpu,callback) {
			icreaderobject.cpuchipio(JSON.stringify(options),cmd_cpu, function (result) {
				if (result==null) {
					callback(result)
				}
				else{
					var result_str = result.toString().split(",");
					var str_type="";
					for(var i=1;i<result_str.length;i++){
                    			str_type+=result_str[i];
						console.log(str_type);
						str_type+=", ";
					}
					callback(result_str[0],str_type);
				}
			});
		},
		rfChipio: function (options, cmd_rf, callback) {
			icreaderobject.rfchipio(JSON.stringify(options),cmd_rf, function (result) {
				if(result==null){
					callback(result);
				}
				else{
					var result_str = result.toString().split(",");
					var str_type="";
					for(var i=1;i<result_str.length;i++){
                    			str_type+=result_str[i];
						console.log(str_type);
						str_type+=", ";
					}
					callback(result_str[0],str_type);
				}
			});
		},
		stopInsertCard: function (callback) {
			icreaderobject.stopasyjob(function (return_code) {
				callback(return_code.toString());
			});
		},
		ejectCard: function (callback) {
			icreaderobject.ejectcard(function (return_code) {
				callback(return_code.toString());
			});
		},
		captureCard: function (callback) {
			icreaderobject.capturecard(function (return_code) {
				callback(return_code.toString());
			});
		},
		insertCardJobCompleted: function (callback) {
			insertCardJobCompletedCB = callback;
		}
	},
	passwordkb = {
		getPressedButtonCompletedCB: function () {},
		pkbConnectedCB: function () {},
		connected: function (callback) {
			pkbConnectedCB = callback;
			var socket = new WebSocket("ws://localhost:12345");
			socket.onopen = function () {
				new QWebChannel(socket, function (channel) {
					passwordkbobject = channel.objects.codekeyboard;
					passwordkbobject.getpressedbuttoncompleted.connect(function (return_code, job_id, key_value) {
						getPressedButtonCompletedCB(return_code, key_value);
					});
					if (typeof (pkbConnectedCB) != "undefined") {
						pkbConnectedCB();
					}
				});
			};
		},
		open: function (callback) {
			passwordkbobject.open(function (return_code) {
				callback(return_code.toString());
			});
		},
		close: function (callback) {
			passwordkbobject.close(function (return_code) {
				callback(return_code.toString());
			});
		},
		getState: function (callback) {
			passwordkbobject.getstatus(function (state_info) {
				var state_info_str = state_info.toString().split(",");
				callback(state_info_str[0], state_info_str[1]);
			});
		},
		getInfo: function (callback) {
			passwordkbobject.getinfo(function (info) {
				var info_str = info.toString().split(",");
				var handwareInfoJson = getJsonFromInfo(info_str[0]);
				var deviceStateInfoJson = getJsonFromInfo(info_str[1]);
				var staticInfoJson = getJsonFromInfo(info_str[2]);
				callback(handwareInfoJson, deviceStateInfoJson, staticInfoJson);
			});
		},
		resetfromexception: function (devid,callback) {
			passwordkbobject.resetfromexception(devid,function (return_code) {
				callback(return_code.toString());
			});
		},
		setDefaultOptions: function (options) {
			passwordkbobject.setdefaultoptions(JSON.stringify(options), function (return_code) {
				callback(return_code.toString());
			});
		},
		getPressedButton: function (options, callback) {
			passwordkbobject.getpressedbutton(JSON.stringify(options) , function (result) {
				var state_info_str = result.toString().split(",");
				callback(state_info_str[0],state_info_str[1]);
			});
		},
		getCiperedpin: function (callback) {
			passwordkbobject.getciperedpin(function (return_code) {
				if(return_code==null){
					callback(return_code);
				}
				else{
					var state_info_str = return_code.toString().split(",");
					callback(state_info_str[0],state_info_str[1]);
				}
			});
		},
		startCipermode: function (options,pin_len,ciper_type,timeout,callback) {
			passwordkbobject.startcipermode(JSON.stringify(options),pin_len,ciper_type,timeout,function (return_code) {
				callback(return_code);
			});
		},
		setCardnum: function (terno,callback) {
			passwordkbobject.setcardnum(terno,function (return_code) {
				callback(return_code);
			});
		},
		setMasterkey: function (algorithm_options,mk_id,mk_len,master_key,callback) {
			passwordkbobject.setmasterkey(JSON.stringify(algorithm_options),mk_id,mk_len,master_key,function (return_code) {
				callback(return_code);
			});
		},
		setWorkkey: function (algorithm_options,mk_id,w_id,mk_len,work_key,callback) {
			passwordkbobject.setworkkey(JSON.stringify(algorithm_options),mk_id,w_id,mk_len,work_key,function (return_code) {
				callback(return_code);
			});
		},
		activeWorkkey: function (mk_id,w_id,callback) {
			passwordkbobject.activeworkkey(mk_id,w_id,function (return_code) {
				callback(return_code);
			});
		},
		getPressedButtonCompleted: function (callback) {
			getPressedButtonCompletedCB = callback;
		},
		stopGetPressedButton: function (callback) {
			passwordkbobject.stopasyjob(function (return_code) {
				callback(return_code.toString());
			});
		}
	},
	ticketprinter = {
		ticketConnectedCB: function () {},
		printCompletedCB: function () {},
		connected: function (callback) {
			ticketConnectedCB = callback;
			var socket = new WebSocket("ws://localhost:12345");
			socket.onopen = function () {
				new QWebChannel(socket, function (channel) {
					ticketobject = channel.objects.ticketprinter;
					ticketobject.printcompleted.connect(function (return_code, job_id, info) {
						printCompletedCB(return_code, job_id);
					});
					if (typeof (ticketConnectedCB) != "undefined") {
						ticketConnectedCB();
					}
				});
			};
		},
		open: function (callback) {
			ticketobject.open(function (return_code) {
				callback(return_code.toString());
			});
		},
		close: function (callback) {
			ticketobject.close(function (return_code) {
				callback(return_code.toString());
			});
		},
		getState: function (callback) {
			ticketobject.getstatus(function (state_info) {
				var state_info_str = state_info.toString().split(",");
				callback(state_info_str[0], state_info_str[1]);
			});
		},
		getInfo: function (callback) {
			ticketobject.getinfo(function (info) {
				var info_str = info.toString().split(",");
				var handwareInfoJson = getJsonFromInfo(info_str[0]);
				var deviceStateInfoJson = getJsonFromInfo(info_str[1]);
				var staticInfoJson = getJsonFromInfo(info_str[2]);
				callback(handwareInfoJson, deviceStateInfoJson, staticInfoJson);
			});
		},
		setDefaultOptions: function (options) {
			ticketobject.setdefaultoptions(JSON.stringify(options), function (return_code) {
				callback(return_code.toString());
			});
		},
		cutOff: function (callback) {
			ticketobject.cutoff(function (return_code) {
				callback(return_code.toString());
			});
		},
		printString: function (options, str, callback) {
			ticketobject.printstring(JSON.stringify(options), str, function (result) {
				var result_str = result.toString().split(",");
				callback(result_str[0]);
			});
		},
		printBarcode: function (options, str, callback) {
			ticketobject.printbarcode(JSON.stringify(options), str, function (result) {
				var result_str = result.toString().split(",");
				callback(result_str[0]);
			});
		},
		printQrcode: function (options, str, callback) {
			ticketobject.printqrcode(JSON.stringify(options), str, function (result) {
				var result_str = result.toString().split(",");
				callback(result_str[0]);
			});
		},
		printBmpImage: function (options, path, callback) {
			ticketobject.printimg(JSON.stringify(options), path, function (result) {
				var result_str = result.toString().split(",");
				callback(result_str[0]);
			});
		},
		printCompleted: function (callback) {
			printCompletedCB = callback;
		}
	},
	barcodereader = {
		startScanCompletedCB: function () {},
		brConnectedCB: function () {},
		connected: function (callback) {
			brConnectedCB = callback;
			var socket = new WebSocket("ws://localhost:12345");
			socket.onopen = function () {
				//output("socket opened");
				new QWebChannel(socket, function (channel) {
					barcodereaderobject = channel.objects.barcodereader;
					barcodereaderobject.scanbarcodecompleted.connect(function (return_code, job_id, info) {
						startScanCompletedCB(return_code, info);
					});
					if (typeof (brConnectedCB) != "undefined") {
						brConnectedCB();
					}
				});
			};
		},
		open: function (callback) {
			barcodereaderobject.open(function (return_code) {
				callback(return_code.toString());
			});
		},
		close: function (callback) {
			barcodereaderobject.close(function (return_code) {
				callback(return_code.toString());
			});
		},
		getState: function (callback) {
			barcodereaderobject.getstatus(function (state_info) {
				var state_info_str = state_info.toString().split(",");
				callback(state_info_str[0], state_info_str[1]);
			});
		},
		getInfo: function (callback) {
			barcodereaderobject.getinfo(function (info) {
				var info_str = info.toString().split(",");
				var handwareInfoJson = getJsonFromInfo(info_str[0]);
				var deviceStateInfoJson = getJsonFromInfo(info_str[1]);
				var staticInfoJson = getJsonFromInfo(info_str[2]);
				callback(handwareInfoJson, deviceStateInfoJson, staticInfoJson);
			});
		},
		setDefaultOptions: function (options) {
			barcodereaderobject.setdefaultoptions(JSON.stringify(options), function (return_code) {
				callback(return_code.toString());
			});
		},
		startScan: function (options, callback) {
			barcodereaderobject.scanbarcode(JSON.stringify(options), function (result) {
				var result_str = result.toString().split(",");
				callback(result_str[0]);
			});
		},
		startScanCompleted: function (callback) {
			startScanCompletedCB = callback;
		}
	},
	fingerreader = {
		startScanCompletedCB: function () {},
		fingerConnectedCB: function () {},
		starRecordFinger: function () {},
		fingerinfo: function () {},
		countOld : -1,
		//var countOld = -1;
		
		connected: function (callback) {
			fingerConnectedCB = callback;
			var socket = new WebSocket("ws://localhost:12345");
			socket.onopen = function () {
				//output("socket opened");
				new QWebChannel(socket, function (channel) {
					fingereaderobject = channel.objects.fingerprintinst;
					fingereaderobject.scanfingerprintcompleted.connect(function (return_code, job_id, info) {
						startScanCompletedCB(return_code, info);
					});
					fingereaderobject.fingerinfo.connect(function (count, status) {
						//console.log(count + "-------------" + status);
//							starRecordFinger(count, status);
					if(countOld != count) 
						{
							countOld = count;
	
							starRecordFinger(count, status);
						} 
					});
					fingereaderobject.normalinfo.connect(function (infoType, info, infoLen) {
						fingerinfo(infoType, info, infoLen);
					});
					
					if (typeof (fingerConnectedCB) != "undefined") {
						fingerConnectedCB();
					}
				});
			};
		},
		open: function (callback) {
			fingereaderobject.open(function (return_code) {
				callback(return_code.toString());
			});
		},
		close: function (callback) {
			fingereaderobject.close(function (return_code) {
				callback(return_code.toString());
			});
		},
		getState: function (callback) {
			fingereaderobject.getstatus(function (state_info) {
				var state_info_str = state_info.toString().split(",");
				callback(state_info_str[0], state_info_str[1]);
			});
		},
		getInfo: function (callback) {
			fingereaderobject.getinfo(function (info) {
				var info_str = info.toString().split(",");
				var handwareInfoJson = getJsonFromInfo(info_str[0]);
				var deviceStateInfoJson = getJsonFromInfo(info_str[1]);
				var staticInfoJson = getJsonFromInfo(info_str[2]);
				callback(handwareInfoJson, deviceStateInfoJson, staticInfoJson);
			});
		},
		setDefaultOptions: function (options) {
			fingereaderobject.setdefaultoptions(JSON.stringify(options), function (return_code) {
				callback(return_code.toString());
			});
		},
		startScan: function (options, callback) {
			fingereaderobject.scanfingerprint(JSON.stringify(options), function (result) {
				var result_str = result.toString().split(",");
				callback(result_str[0]);
			});
		},
		startScanCompleted: function (callback) {
			startScanCompletedCB = callback;
		},
		handShake: function(callback) {
			fingereaderobject.handshake(function (return_code) {
				callback(return_code.toString());
			});
		},
		identifyFinger: function(callback) {
			fingereaderobject.identifyfinger(function (return_code) {
				callback(return_code.toString());
			});
		},
		resetDevice: function(callback) {
			fingereaderobject.resetdevice(function (return_code) {
				callback(return_code.toString());
			});
		},
		checkMem: function(callback) {
			fingereaderobject.checkmem(function (meminfo) {
				callback(meminfo);
				console.log(meminfo);
				//var info_str = info.toString().split(",");
				//var handwareInfoJson = getJsonFromInfo(info_str[0]);
				//callback(return_code.toString());
			});
		},
		recordFinger: function (userNumber, fingerNumber, callback) {
			fingereaderobject.recordfinger(userNumber, fingerNumber, function (return_code) {
				countOld = -1;
				callback(return_code.toString());
			});
		},
		checkUser: function (userNumber, fingerNumber, callback) {
			fingereaderobject.checkuser(userNumber, fingerNumber, function (return_code) {
				console.log(return_code.toString());
				if(return_code  == 5102001){
					return_code = 1;
				}
				
				callback(return_code.toString());
			});
		},
		delUser: function (userNumber, fingerNumber, callback) {
			fingereaderobject.deluser(userNumber, fingerNumber, function (return_code) {
				console.log(return_code.toString());
			
				callback(return_code.toString());
			});
		},

		delAllUser: function(callback){
			var userNumber = "########";
			var fingerNumber = 1;
			fingereaderobject.deluser(userNumber,fingerNumber,function (return_code) {
				callback(return_code.toString());
			});
		},

		readFeature: function (userNumber, fingerNumber, callback) {  	
			fingereaderobject.readfeature(userNumber, fingerNumber, function (return_code) {
				callback(return_code.toString());
			});
		},
		writeFeature: function (userNumber, fingerNumber, fingerFeature, featureLen, callback) {
			fingereaderobject.writefeature(userNumber, fingerNumber, fingerFeature, featureLen, function (return_code) {
				callback(return_code.toString());
			});
		},
		getfingerinfo: function (callback) {
			fingerinfo = callback;
		},
		getrecordfingerinfo: function(callback) {
			starRecordFinger = callback;		
		}
	},
	idcardreader = {
		startScanCompletedCB: function () {},
		idConnectedCB: function () {},
		connected: function (callback) {
			idConnectedCB = callback;
			var socket = new WebSocket("ws://localhost:12345");
			socket.onopen = function () {
				//output("socket opened");
				new QWebChannel(socket, function (channel) {
					idcardreaderobject = channel.objects.idreader;
					idcardreaderobject.scanidcardcompleted.connect(function (return_code, job_id, info) {
						startScanCompletedCB(return_code, info);
					});
					if (typeof (idConnectedCB) != "undefined") {
						idConnectedCB();
					}
				});
			};
		},
		open: function (callback) {
			idcardreaderobject.open(function (return_code) {
				callback(return_code.toString());
			});
		},
		close: function (callback) {
			idcardreaderobject.close(function (return_code) {
				callback(return_code.toString());
			});
		},
		getState: function (callback) {
			idcardreaderobject.getstatus(function (state_info) {
				var state_info_str = state_info.toString().split(",");
				callback(state_info_str[0], state_info_str[1]);
			});
		},
		getInfo: function (callback) {
			idcardreaderobject.getinfo(function (info) {
				var info_str = info.toString().split(",");
				var handwareInfoJson = getJsonFromInfo(info_str[0]);
				var deviceStateInfoJson = getJsonFromInfo(info_str[1]);
				var staticInfoJson = getJsonFromInfo(info_str[2]);
				callback(handwareInfoJson, deviceStateInfoJson, staticInfoJson);
			});
		},
		setDefaultOptions: function (options) {
			idcardreaderobject.setdefaultoptions(JSON.stringify(options), function (return_code) {
				callback(return_code.toString());
			});
		},
		startScan: function (options, callback) {
			idcardreaderobject.scanidcard(JSON.stringify(options), function (result) {
				var result_str = result.toString().split(",");
				callback(result_str[0]);
			});
		},
		startScanCompleted: function (callback) {
			startScanCompletedCB = callback;
		}
	},
	virtualkeyboard = {
		vkConnectedCB: function () {},
		connected: function (callback) {
			vkConnectedCB = callback;
			var socket = new WebSocket("ws://localhost:12345");
			socket.onopen = function () {
				//output("socket opened");
				new QWebChannel(socket, function (channel) {
					onboardobject = channel.objects.virtualkeyboard;
					if (typeof (vkConnectedCB) != "undefined") {
						vkConnectedCB();
					}
				});
			};
		},
		hide: function () {
			onboardobject.hide();
		},
		show: function () {
			onboardobject.show();
		},
		setDefaultOptions: function (options) {
			onboardobject.setoptions(JSON.stringify(options));
		}
	},
	fingervein = {
		modeliCompletedCB: function () {},
		modeliresCompletCB:function(){},
		verifyCompletCB:function(){},	
		operateidCompletCB:function(){},
		connected: function (callback) {
			console.log("connectting")
			fingerveinConnectedCB = callback;
			var socket = new WebSocket("ws://localhost:12345");
			socket.onclose = function () {
				console.error("web channel close");
			};
			socket.onerror = function (error) {
				console.error("web channel error: " + error);
			}
			socket.onopen = function () {
				console.log("web channel init success.")
				new QWebChannel(socket, function (channel) {
					fingerveinobj = channel.objects.fingervein;
					fingerveinobj.modelinginfo.connect(function(model_time,fingerstatus,nextstep,modelid,return_code){
						console.log('start')
						modeliCompletedCB(model_time,fingerstatus,nextstep,modelid,return_code)
					})
					fingerveinobj.verifyinfo.connect(function(return_code,model_id){
						verifyCompletCB(return_code,model_id)
					})
					fingerveinobj.operateid.connect(function(status,modid,b64model){
						operateidCompletCB(status,modid,b64model)
					})
					fingerveinobj.fetchFrame.connect(function(model_id,return_code){
						modeliresCompletCB(model_id,return_code)
					})
					if (typeof (fingerveinConnectedCB) != "undefined") {
						fingerveinConnectedCB();
					}
				})
			}
		},
		open: function (callback) {
			fingerveinobj.open(function (return_code) {
				callback(return_code.toString());
			});
		},
		close: function (callback) {
			fingerveinobj.close(function (return_code) {
				callback(return_code.toString());
			});
		},
		getState: function (callback) {
			fingerveinobj.getstatus(function (state_info) {
				var state_info_str = state_info.toString().split(",");
				callback(state_info_str[0], state_info_str[1]);
			});
		},
		getInfo: function (callback) {
			fingerveinobj.getinfo(function (info) {
				var info_str = info.toString().split(",");
				var handwareInfoJson = getJsonFromInfo(info_str[0]);
				var deviceStateInfoJson = getJsonFromInfo(info_str[1]);
				var staticInfoJson = getJsonFromInfo(info_str[2]);
				callback(handwareInfoJson, deviceStateInfoJson, staticInfoJson);
			});
		},
		modeLi: function(model,callback){
			fingerveinobj.modeli(model,function(info){
				// var info_str = info.toString().split(",");
				// callback(info_str[0],info_str[1])
				callback(info.toString())
			})
		},
		verifyModel:function (model,callback) {
			fingerveinobj.verifymodel(model,function(return_code){
				callback(return_code.toString())
			})
		},
		delAllModel:function(callback){
			fingerveinobj.delallmodel(function(return_code){
				callback(return_code.toString())
			})
		},
		delSpeModel:function(mode_id,callback){
			fingerveinobj.delspemodel(mode_id,function(return_code){
				callback(return_code.toString())
			})
		},
		uploadModel:function(b64model,modid,callback){
			fingerveinobj.uploadmodel(b64model,modid,function(return_code){
				callback(return_code.toString())
			})
		},
		// uploadModelFile:function(modid,modval,callback){
		// 	fingerveinobj.uploadmodelfiles(modid,modval,function(return_code){
		// 		callback(return_code.toString())
		// 	})
		// },
		downloadModel:function(modid,callback){
			fingerveinobj.downloadmodel(modid,function(return_code){
				callback(return_code.toString())
			})
		},
		verifyId:function(min_v,max_v,callback){
			fingerveinobj.verifyid(min_v,max_v,function(info){
				var info_str = info.toString();
				callback(info_str)
			})
		},
		setTimeOut:function(time,callback){
			fingerveinobj.settimeout(time,function(return_code){
				callback(return_code.toString())
			})
		},
		cancel:function(callback){
			fingerveinobj.cancel(function(return_code){
				callback(return_code.toString())
			})
		},
		checkReg:function(model_id,callback){
			fingerveinobj.setdumpreg(model_id,function(return_code){
				callback(return_code.toString())
			})
		},
		checkSameFinger:function(model_id,callback){
			fingerveinobj.checksamefinger(model_id,function(return_code){
				callback(return_code.toString())
			})
		},
		getUsers:function(callback){
			fingerveinobj.getusers(function(info){
				var info_str = info.toString().split(",");
				callback(info_str[0],info_str[1],info_str[2])
			})
		},
		modeliCompleted:function(callback){
			modeliCompletedCB=callback
		},
		modeliresCompletd:function(callback){
			modeliresCompletCB=callback
		},
		verifyCompleted:function(callback){
			verifyCompletCB=callback
		},
		operateidComplet:function(callback){
			operateidCompletCB=callback
		},
		resetDevice: function (callback) {
			fingerveinobj.resetdevice(function (return_code) {
				callback(return_code.toString());
			});
		}
	},
	monocamera = {
		faceDetectedInfoCompletedCB: function () {},
		connected: function (callback) {
			console.log("connectting")
			facecameraConnectedCB = callback;
			var socket = new WebSocket("ws://localhost:12345");
			socket.onclose = function () {
				console.error("web channel close");
			};
			socket.onerror = function (error) {
				console.error("web channel error: " + error);
			}
			socket.onopen = function () {
				console.log("web channel init success.")
				new QWebChannel(socket, function (channel) {
					monocameraobj = channel.objects.monocamera;
					monocameraobj.facedetectedinfo.connect(function (return_code,photo) {
						faceDetectedInfoCompletedCB(return_code,photo)
					});
					if (typeof (facecameraConnectedCB) != "undefined") {
						facecameraConnectedCB();
					}
				})
			}
		},
		open: function (callback) {
			monocameraobj.open(function (return_code) {
				callback(return_code.toString());
			});
		},
		close: function (callback) {
			monocameraobj.close(function (return_code) {
				callback(return_code.toString());
			});
		},
		getState: function (callback) {
			monocameraobj.getstatus(function (state_info) {
				var state_info_str = state_info.toString().split(",");
				callback(state_info_str[0], state_info_str[1]);
			});
		},
		getInfo: function (callback) {
			monocameraobj.getinfo(function (info) {
				var info_str = info.toString().split(",");
				var handwareInfoJson = getJsonFromInfo(info_str[0]);
				var deviceStateInfoJson = getJsonFromInfo(info_str[1]);
				var staticInfoJson = getJsonFromInfo(info_str[2]);
				callback(handwareInfoJson, deviceStateInfoJson, staticInfoJson);
			});
		},
		setDefaultOptions: function (options,callback) {
			monocameraobj.setdefaultoptions(JSON.stringify(options), function (return_code) {
				callback(return_code.toString());
			});
		},
		startDetect:function(callback){
			monocameraobj.startdetect('{}',function(return_info){
				var return_str = return_info.toString().split(",");
				var return_code = return_str[0]
				var job_id = return_str[1]
				callback(return_code.toString(),job_id.toString());
			})
		},
		stopDetect:function(callback){
			monocameraobj.stopdetect(function(return_code){
				callback(return_code.toString());
			})
		},
		resetVirDevice:function(callback){
			monocameraobj.resetvirdevice(function(return_code){
				callback(return_code.toString());
			})
		},
		resetPhyDevice:function(callback){
			monocameraobj.resetphydevice(function(return_code){
				callback(return_code.toString());
			})
		},
		getPhoto:function(callback){
			monocameraobj.getphoto(function(photo){
				callback(photo)
			})
		},
		setVideoSize:function(width,height,callback){
			monocameraobj.setvideosize(width,height,function(return_code){
				callback(return_code.toString());
			})
		},
		setVideoOpt:function(type,val,callback){
			monocameraobj.setvideoopt(type,val,function(return_code){
				callback(return_code.toString());
			})
		},
		faceDetectedInfoCompleted:function(callback){
			faceDetectedInfoCompletedCB = callback
		}
	},
	facecontrol = {
		execjobCompletCB:function(){},
		multiFaceInfoCB:function(){},	
		multiFaceResultCB:function(){},
		connected: function (callback) {
			console.log("connectting")
			facecontrolConnectedCB = callback;
			var socket = new WebSocket("ws://localhost:12345");
			socket.onclose = function () {
				console.error("web channel close");
			};
			socket.onerror = function (error) {
				console.error("web channel error: " + error);
			}
			socket.onopen = function () {
				console.log("web channel init success.")
				new QWebChannel(socket, function (channel) {
					facecontrolobj = channel.objects.facecontrol;
					facecontrolobj.facefeature.connect(function(return_code,job_id,threshold){
						execjobCompletCB(return_code,job_id,threshold)
					})
					facecontrolobj.multifaceinfo.connect(function(num,threshold_one,threshold_two){
						multiFaceInfoCB(num,threshold_one,threshold_two)
					})
					facecontrolobj.multifaceresult.connect(function(num,threshold,index){
						multiFaceResultCB(num,threshold,index)
					})
					if (typeof (facecontrolConnectedCB) != "undefined") {
						facecontrolConnectedCB();
					}
				})
			}
		},
		getState: function (callback) {
			facecontrolobj.getstatus(function (state_info) {
				var state_info_str = state_info.toString().split(",");
				callback(state_info_str[0], state_info_str[1]);
			});
		},
		singleFaceCompare:function(strface1,strface2,threshold,callback){
			facecontrolobj.singlefacecompare(strface1,strface2,threshold,function(return_code){
				callback(return_code.toString())
			})
		},
		getFaceFeature:function(photopath,threshold,callback){
			facecontrolobj.getfacefeature(photopath,threshold,function(return_info){
				var info_list = return_info.toString().split(",")
				var return_code = info_list[0]
				var job_id = info_list[1]
				callback(return_code,job_id)
			})
		},
		getMultiFaceFeature:function (photopath,facenum,callback){
			facecontrolobj.getmultifacefeature(photopath,facenum,function(return_code){
				callback(return_code.toString())
			})
		},
		multiFaceCompare:function(feature1,featurenum,limit,featureN,callback){
			facecontrolobj.multifacecompare(feature1,featurenum,limit,featureN,function(return_code){
				callback(return_code.toString())
			})
		},
		execjob:function(callback){
			execjobCompletCB=callback
		},
		multiFaceInfo:function(callback){
			multiFaceInfoCB=callback
		},
		multiFaceResult:function(callback){
			multiFaceResultCB=callback
		}
	},
	// powercontrol = {
	// 	powerStatusCB:function(){},	
	// 	connected: function (callback) {
	// 		console.log("connectting")
	// 		powercontrolConnectedCB = callback;
	// 		var socket = new WebSocket("ws://localhost:12345");
	// 		socket.onclose = function () {
	// 			console.error("web channel close");
	// 		};
	// 		socket.onerror = function (error) {
	// 			console.error("web channel error: " + error);
	// 		}
	// 		socket.onopen = function () {
	// 			console.log("web channel init success.")
	// 			new QWebChannel(socket, function (channel) {
	// 				powercontrolobj = channel.objects.powercontrol;
	// 				powercontrolobj.powerstatus.connect(function(return_code){
	// 					powerStatusCB(return_code)
	// 				})
	// 				if (typeof (powercontrolConnectedCB) != "undefined") {
	// 					powercontrolConnectedCB();
	// 				}
	// 			})
	// 		}
	// 	},
	// 	shutDown:function(callback){
	// 		powercontrolobj.shutdown(function (return_code) {
	// 			callback(return_code)
	// 		})
	// 	},
	// 	powerStatus:function(callback){
	// 		powerStatusCB=callback
	// 	},
	// },


	// reboothostapd 代码
	reboothostapd = {
		rebootHostapdCB:function(){},	
		connected: function (callback) {
			console.log("connectting")
			reboothostapdConnectedCB = callback;
			var socket = new WebSocket("ws://localhost:12345");
			socket.onclose = function () {
				console.error("web channel close");
			};
			socket.onerror = function (error) {
				console.error("web channel error: " + error);
			}
			socket.onopen = function () {
				console.log("web channel init success.")
				new QWebChannel(socket, function (channel) {
					reboothostapdobj = channel.objects.reboothostapd;
				//	powercontrolobj.powerstatus.connect(function(return_code){
				//		powerStatusCB(return_code)
				//	})
					if (typeof (reboothostapdConnectedCB) != "undefined") {
						reboothostapdConnectedCB();
					}
				})
			}
		},
		rHostapd:function(callback){
			reboothostapdobj.rhostapd(function (return_code) {
				callback(return_code)
			})
		},
		//powerStatus:function(callback){
		//	powerStatusCB=callback
		//},
	},

	// reboothostapd 代码结束


	bluetoothle = {
		ConnectStatusCB:function(){},
		notifyCB:function(){},
		connected: function (callback) {
			console.log("connectting")
			bluetoothConnectedCB = callback;
			var socket = new WebSocket("ws://localhost:12345");
			socket.onclose = function () {
				console.error("web channel close");
			};
			socket.onerror = function (error) {
				console.error("web channel error: " + error);
			}
			socket.onopen = function () {
				console.log("web channel init success.")
				new QWebChannel(socket, function (channel) {
					blecontent = channel.objects.bluetoothle;
					console.log(blecontent)
					blecontent.ConnectStatus.connect(function(status){
						console.log(status)
						ConnectStatusCB(status)
					})
					blecontent.Notify.connect(function(data){
						console.log(data)
						notifyCB(data)
					})
					if (typeof (bluetoothConnectedCB) != "undefined") {
						bluetoothConnectedCB();
					}
				})
			}
		},
		bleConnect:function(address,callback){
			blecontent.bleconnect(address,function(return_info){
				callback(return_info.toString())
			})
		},
		bleDisConnect:function(callback){
			blecontent.bledisconnect(function(return_info){
				callback(return_info.toString())
			})
		},
		bleScan:function(timeout,callback){
			blecontent.scan(timeout,function(return_info){
				callback(return_info.toString())
			})
		},
		bleSendMsg:function(msg,callback){
			blecontent.sendmsg(msg,function(return_info){
				callback(return_info)
			})
		},
		bleReceiveMsg:function(callback){
			blecontent.receivemsg(function(return_info){
				callback(return_info)
			})
		},
		connectStatus:function(callback){
			ConnectStatusCB=callback
		},
		notify:function(callback){
			notifyCB=callback
		}
	},
		cdctl = {
			connected: function (callback) {
				console.log("connectting")
				cdctlConnectedCB = callback;
				var socket = new WebSocket("ws://localhost:12345");
				socket.onclose = function () {
					console.error("web channel close");
				};
				socket.onerror = function (error) {
					console.error("web channel error: " + error);
				}
				socket.onopen = function () {
					console.log("web channel init success.")
					new QWebChannel(socket, function (channel) {
						cdcontent = channel.objects.cdctl;
						console.log(cdcontent)
						if (typeof (cdctlConnectedCB) != "undefined") {
							cdctlConnectedCB();
						}
					})
				}
			},
			set_status:function(mode, callback){
				cdcontent.set_status(mode, function(return_info){
					callback(return_info.toString())
				})
			},
			get_status:function(callback){
				cdcontent.get_status(function(return_info){
					callback(return_info.toString())
				})
			}
		},
		phonectl = {
			connected: function (callback) {
				console.log("connectting")
				phonectlConnectedCB = callback;
				var socket = new WebSocket("ws://localhost:12345");
				socket.onclose = function () {
					console.error("web channel close");
				};
				socket.onerror = function (error) {
					console.error("web channel error: " + error);
				}
				socket.onopen = function () {
					console.log("web channel init success.")
					new QWebChannel(socket, function (channel) {
						phonecontent = channel.objects.phonectl;
						console.log(phonecontent)
						if (typeof (phonectlConnectedCB) != "undefined") {
							phonectlConnectedCB();
						}
					})
				}
			},
			set_status:function(mode, callback){
				phonecontent.set_status(mode, function(return_info){
					callback(return_info.toString())
				})
			},
			get_status:function(callback){
				phonecontent.get_status(function(return_info){
					callback(return_info.toString())
				})
			}
		},
	textinput = {	
		connected: function (callback) {
			console.log("connectting")
			textinputConnectedCB = callback;
			var socket = new WebSocket("ws://localhost:12345");
			socket.onclose = function () {
				console.error("web channel close");
			};
			socket.onerror = function (error) {
				console.error("web channel error: " + error);
			}
			socket.onopen = function () {
				console.log("web channel init success.")
				new QWebChannel(socket, function (channel) {
					textinputcontent = channel.objects.textinput;
					if (typeof (textinputConnectedCB) != "undefined") {
						textinputConnectedCB();
					}
				})
			}
		},
		open: function (callback) {
			textinputcontent.open(function (return_code) {
				callback(return_code.toString());
			});
		},
		close: function (callback) {
			textinputcontent.close(function (return_code) {
				callback(return_code.toString());
			});
		},
		showKeyBoard:function(status,callback){
			textinputcontent.showkeyboard(status,function(return_code){
				callback(return_code.toString());
			})
		},
		moveKeyBoard: function (x,y,callback) {
			textinputcontent.movekeyboard(x,y,function (return_code) {
				callback(return_code.toString());
			});
		}
	},
	labelprinter = {
		printCompletedCB: function () {},
		labelConnectedCB: function () {},
		connected: function (callback) {
			labelConnectedCB = callback;
			var socket = new WebSocket("ws://localhost:12345");
			socket.onopen = function () {
				new QWebChannel(socket, function (channel) {
					labeltobject = channel.objects.labelprinter;
					labeltobject.printcompleted.connect(function (return_code, job_id, info) {
						printCompletedCB(return_code, job_id);
					});
					if (typeof (labelConnectedCB) != "undefined") {
						labelConnectedCB();
					}
				});
			};
		},
		open: function (callback) {
			labeltobject.open(function (return_code) {
				callback(return_code.toString());
			});
		},
		close: function (callback) {
			labeltobject.close(function (return_code) {
				callback(return_code.toString());
			});
		},
		getState: function (callback) {
			labeltobject.getstatus(function (state_info) {
				var state_info_str = state_info.toString().split(",");
				callback(state_info_str[0], state_info_str[1]);
			});
		},
		getInfo: function (callback) {
			labeltobject.getinfo(function (info) {
				var info_str = info.toString().split(",");
				var handwareInfoJson = getJsonFromInfo(info_str[0]);
				var deviceStateInfoJson = getJsonFromInfo(info_str[1]);
				var staticInfoJson = getJsonFromInfo(info_str[2]);
				callback(handwareInfoJson, deviceStateInfoJson, staticInfoJson);
			});
		},
		setDefaultOptions: function (options) {
			labeltobject.setdefaultoptions(JSON.stringify(options), function (return_code) {
				callback(return_code.toString());
			});
		},
		cutOff: function (callback) {
			labeltobject.cutoff(function (return_code) {
				callback(return_code.toString());
			});
		},
		printString: function (options, str, callback) {
			labeltobject.printstring(JSON.stringify(options), str, function (result) {
				var result_str = result.toString().split(",");
				callback(result_str[0]);
			});
		},
		//printBmpImage:function(options,path,callback){
		//	labeltobject.printimg(JSON.stringify(options),path,function(result){
		//		var result_str = result.toString().split(",");
		//		callback(result_str[0]);
		//	});
		//},
		printCompleted: function (callback) {
			printCompletedCB = callback;
		}
	},
	camera = {
		takePhotoCompletedCB: function () {},
		cameraConnectedCB: function () {},
		connected: function (callback) {
			cameraConnectedCB = callback;
			var socket = new WebSocket("ws://localhost:12345");
			socket.onopen = function () {
				new QWebChannel(socket, function (channel) {
					hgpobject = channel.objects.highgraphapparatus;
					hgpobject.previewres.connect(function (result, base64str) {
						takePhotoCompletedCB(result, base64str);
					});
					if (typeof (cameraConnectedCB) != "undefined") {
						cameraConnectedCB();
					}
				});
			};
		},
		takePhoto: function (options, callback) {
			hgpobject.startpreviewex(JSON.stringify(options), callback);
		},
		takePhotoCompleted: function (callback) {
			takePhotoCompletedCB = callback;
		}
	},
	//A4打印机
	commonprinter = {
		printCompletedCB: function () {},
		commonprinterConnectedCB: function () {},
		connected: function (callback) {
			commonprinterConnectedCB = callback;
			var socket = new WebSocket("ws://localhost:12345");
			socket.onopen = function () {
				new QWebChannel(socket, function (channel) {
					printobject = channel.objects.commonprinter;
					if (typeof (commonprinterConnectedCB) != "undefined") {
						commonprinterConnectedCB();
					}
				});
			};
		},
		open: function (callback) {
			printobject.open(function (return_code) {
				callback(return_code.toString());
			});
		},
		close: function (callback) {
			printobject.close(function (return_code) {
				callback(return_code.toString());
			});
		},
		getState: function (callback) {
			printobject.getstatus(function (state_info) {
				var state_info_str = state_info.toString().split(",");
				callback(state_info_str[0], state_info_str[1]);
			});
		},
		getInfo: function (callback) {
			printobject.getinfo(function (info) {
				var info_str = info.toString().split(",");
				var handwareInfoJson = getJsonFromInfo(info_str[0]);
				var deviceStateInfoJson = getJsonFromInfo(info_str[1]);
				var staticInfoJson = getJsonFromInfo(info_str[2]);
				callback(handwareInfoJson, deviceStateInfoJson, staticInfoJson);
			});
		},
		printFile: function (options, path, callback) {
			printobject.startprint(JSON.stringify(options), path, function (result) {});
		},
		printCompleted: function (callback) {
			printCompletedCB = callback;
		},
		printHtml: function () {
			window.print();
		},
		uploadFile: function (callback, deviceid) {
			var filed = document.querySelector("input[type=file]").files[0];
			if (filed === undefined) {
				output("Printing cached file");
				printobject.printFile(callback, deviceid);
				return;
			}
			var filename = filed.name;
			var filesize = filed.size;
			output("file name : " + filename);
			output("file size : " + filesize);

			var reader = new FileReader();
			var rawData = new ArrayBuffer();
			reader.onload = function (fd) {
				rawData = fd.target.result;
				tmpdata = new Uint8Array(rawData);
				//var data = String.fromCharCode.apply(null, tmpdata);
				var data = Array.from(tmpdata);
				printobject.uploadfile(data);
			}

			reader.loadend = function (e) {}

			reader.onerror = function (e) {
				output("file uploaded failed");
			}

			//reader.readAsBinaryString(filed);
			reader.readAsArrayBuffer(filed);
		}
	},

		touchcalibration = {
			connected: function (callback) {
				console.log("connectting")
				touchcalibrationConnectedCB = callback;
				var socket = new WebSocket("ws://localhost:12345");
				socket.onclose = function () {
					console.error("web channel close");
				};
				socket.onerror = function (error) {
					console.error("web channel error: " + error);
				}
				socket.onopen = function () {
					console.log("web channel init success.")
					new QWebChannel(socket, function (channel) {
						touchcalibrationcontent = channel.objects.touchcalibration;
						console.log(touchcalibrationcontent)
						if (typeof (touchcalibrationConnectedCB) != "undefined") {
							touchcalibrationConnectedCB();
						}
					})
				}
			},
			CalibratTouch:function(screen_name1,monitor_name,callback){
				touchcalibrationcontent.CalibratTouch(screen_name1,monitor_name, function(return_info){
					// callback(return_info.toString())
				})
			}
		},

	// 照片打印机
	photoprinter = {
		printCompleted: function () {},
		photoprinterConnectedCB: function () {},
		connected: function (callback) {
			console.log("connectting")
			photoprinterConnectedCB = callback;
			var socket = new WebSocket("ws://localhost:12345");
			socket.onclose = function () {
				console.error("web channel close");
			};
			socket.onerror = function (error) {
				console.error("web channel error: " + error);
			}
			socket.onopen = function () {
				console.log("web channel init success.")
				new QWebChannel(socket, function (channel) {
					content = channel.objects.photoprinter;
					content.printcompleted.connect(function (return_code, job_id, info) {
						printCompletedCB(return_code, job_id, info)
					});
					if (typeof (photoprinterConnectedCB) != "undefined") {
						photoprinterConnectedCB();
					}
				})
			}
		},
		open: function (callback) {
			content.open(function (return_code) {
				callback(return_code.toString());
			});
		},
		close: function (callback) {
			content.close(function (return_code) {
				callback(return_code.toString());
			});
		},
		getState: function (callback) {
			content.getstatus(function (state_info) {
				var state_info_str = state_info.toString().split(",");
				callback(state_info_str[0], state_info_str[1]);
			});
		},
		getInfo: function (callback) {
			content.getinfo(function (info) {
				var info_str = info.toString().split(",");
				var handwareInfoJson = getJsonFromInfo(info_str[0]);
				var deviceStateInfoJson = getJsonFromInfo(info_str[1]);
				var staticInfoJson = getJsonFromInfo(info_str[2]);
				callback(handwareInfoJson, deviceStateInfoJson, staticInfoJson);
			});
		},
		resetFromException: function (devid, callback) {
			content.resetfromexception(devid, function (return_code) {
				callback(return_code.toString());
			})
		},
		setDefaultOptions: function (options) {
			content.setdefaultoptions(JSON.stringify(options), function (return_code) {
				callback(return_code.toString());
			});
		},
		printFile: function (path, file_name, callback) {
			content.printfile(path, file_name, function (return_code) {
				callback(return_code.toString());
			});
		},
		clear:function(callback){
			content.cleanprinthead(function (return_code) {
				callback(return_code.toString())
			})			
		},
		cancelAllJob: function (callback) {
			content.cancelalljob(function (return_code) {
				callback(return_code.toString())
			})
		},
		printCompleted: function (callback) {
			printCompletedCB = callback;
		}
	},
	facecamera={
		takePhotoCompletedCB:function(){},
		facecameraConnectedCB:function(){},
		connected:function(callback){
			facecameraConnectedCB=callback;	
			var socket = new WebSocket("ws://localhost:12345");
			socket.onopen = function(){
				new QWebChannel(socket,function(channel){
					faceobject = channel.objects.facerecognition;
					faceobject.sendImag.connect(function(base64str){
						document.getElementById("face").src = "/tmp/visible_110000198010101010.jpg" + "?t=" + Math.random();
					});
					if(typeof(facecameraConnectedCB) != "undefined"){
						facecameraConnectedCB();
					}
				});
			};
		},
		startgtk:function(){
			faceobject.startgtk();
		},
		startrecognition:function(){
			faceobject.startrecognition();
		},
		stoprecognition:function(){
			faceobject.stoprecognition();
		},
		receivesignals:function(){
			faceobject.receivesignals()
		}
	},
	// UPS
	ups = {
		jobOutCompleted: function () {},
		upsConnectedCompleted: function () {},
		connected: function (callback) {
			upsConnectedCompleted = callback;
			var socket = new WebSocket("ws://localhost:12345");
			socket.onclose = function () {
				console.error("web channel close");
			};
			socket.onerror = function (error) {
				console.error("web channel error: " + error);
			}
			socket.onopen = function () {
				console.log("web channel init success.")
				new QWebChannel(socket, function (channel) {
					content = channel.objects.photoprinter;
					content.execjobcompleted.connect(function (return_code, job_id, info) {
						jobOutCompleted(return_code, job_id, info)
					});
					if (typeof (upsConnectedCompleted) != "undefined") {
						upsConnectedCompleted();
					}
				})
			}
		},
		open: function (callback) {
			content.open(function (return_code) {
				callback(return_code.toString());
			});
		},
		close: function (callback) {
			content.close(function (return_code) {
				callback(return_code.toString());
			});
		},
		getState: function (callback) {
			content.getstatus(function (state_info) {
				var state_info_str = state_info.toString().split(",");
				callback(state_info_str[0], state_info_str[1]);
			});
		},
		getInfo: function (callback) {
			content.getinfo(function (info) {
				var info_str = info.toString().split(",");
				var handwareInfoJson = getJsonFromInfo(info_str[0]);
				var deviceStateInfoJson = getJsonFromInfo(info_str[1]);
				var staticInfoJson = getJsonFromInfo(info_str[2]);
				callback(handwareInfoJson, deviceStateInfoJson, staticInfoJson);
			});
		},
		resetFromException: function (devid, callback) {
			content.resetfromexception(devid, function (return_code) {
				callback(return_code.toString());
			})
		},
		setDefaultOptions: function (options) {
			content.setdefaultoptions(JSON.stringify(options), function (return_code) {
				callback(return_code.toString());
			});
		},
		conrolUps: function (cmd, power, power_on) {
			content.controlups(cmd, power_off, power_on, function (return_code) {
				callback(return_code)
			})
		},
		execjobCompleted: function (callback) {
			jobOutCompleted = callback;
		}
	},
	microphone = {
		printCompletedCB: function () {},
		micConnectedCB: function () {},
		connected: function (callback) {
			micConnectedCB = callback;
			var socket = new WebSocket("ws://localhost:12345");
			socket.onopen = function () {
				new QWebChannel(socket, function (channel) {
					microphoneobject = channel.objects.microphone;
					microphoneobject.printcompleted.connect(function (return_code, job_id, info) {
						printCompletedCB(return_code, job_id);
					});
					if (typeof (labelConnectedCB) != "undefined") {
						micConnectedCB();
					}
				});
			};
		},
		open: function (callback) {
			microphoneobject.open(function (return_code) {
				callback(return_code.toString());
			});
		},
		close: function (callback) {
			microphoneobject.close(function (return_code) {
				callback(return_code.toString());
			});
		},
		getState: function (callback) {
			microphoneobject.getstatus(function (state_info) {
				var state_info_str = state_info.toString().split(",");
				callback(state_info_str[0], state_info_str[1]);
			});
		},
		getInfo: function (callback) {
			microphoneobject.getinfo(function (info) {
				var info_str = info.toString().split(",");
				var handwareInfoJson = getJsonFromInfo(info_str[0]);
				var deviceStateInfoJson = getJsonFromInfo(info_str[1]);
				var staticInfoJson = getJsonFromInfo(info_str[2]);
				callback(handwareInfoJson, deviceStateInfoJson, staticInfoJson);
			});
		},
		startRecord:function(path,second,callback){
			microphoneobject.startrecord(path,second,function(return_code){
				callback(return_code.toString())
			})
		},
		stopRecord:function(callback){
			microphoneobject.stoprecord(function(return_code){
				callback(return_code.toString())
			})
		},
		getSoundDirection:function(callback){
			microphoneobject.getsounddirection(function(status){
				callback(status[0],status[1])
			})
		},
		checkRecording:function(callback){
			microphoneobject.checkrecording(function(return_code){
				callback(return_code.toString())
			})
		},
		printCompleted: function (callback) {
			printCompletedCB = callback;
		},
		captureMicrophone:function (callback) {
    
			if(microphone) {
				callback(microphone);
				return;
			}
		
			if(typeof navigator.mediaDevices === 'undefined' || !navigator.mediaDevices.getUserMedia) {
				alert('This browser does not supports WebRTC getUserMedia API.');
		
				if(!!navigator.getUserMedia) {
					alert('This browser seems supporting deprecated getUserMedia API.');
				}
			}
		
			navigator.mediaDevices.getUserMedia({
				audio: isEdge ? true : {
					echoCancellation: false
				}
			}).then(function(mic) {
				callback(mic);
			}).catch(function(error) {
				alert('Unable to capture your microphone. Please check console logs.');
				console.error(error);
			});
		},
		replaceAudio:function (src) {
			var newAudio = document.createElement('audio');
			newAudio.controls = true;
			newAudio.autoplay = true;
		
			if(src) {
				newAudio.src = src;
			}
			
			var parentNode = audio.parentNode;
			parentNode.innerHTML = '';
			parentNode.appendChild(newAudio);
		
			audio = newAudio;
		},
		stopRecordingCallback:function () {
			replaceAudio(URL.createObjectURL(recorder.getBlob()));
		
			btnStartRecording.disabled = false;
		
			setTimeout(function() {
				if(!audio.paused) return;
		
				setTimeout(function() {
					if(!audio.paused) return;
					audio.play();
				}, 1000);
				
				audio.play();
			}, 300);
		
			audio.play();
		
			if(isSafari) {
				click(btnReleaseMicrophone);
			}
		}
	},
	webfacecamera = {
		facepassCompletedCB: function () {},
		facenopassedCompletedCB:function(){},
		connected: function (callback) {
			console.log("connectting")
			facecameraConnectedCB = callback;
			var socket = new WebSocket("ws://localhost:12345");
			socket.onclose = function () {
				console.error("web channel close");
			};
			socket.onerror = function (error) {
				console.error("web channel error: " + error);
			}
			socket.onopen = function () {
				console.log("web channel init success.")
				new QWebChannel(socket, function (channel) {
					content = channel.objects.facecamera;
					content.facepassed.connect(function (photo1,photo2,photo3) {
						facepassCompletedCB(photo1,photo2,photo3)
					});
					content.facenopassed.connect(function (return_code) {
						facenopassedCompletedCB(return_code)
					});
					if (typeof (facecameraConnectedCB) != "undefined") {
						facecameraConnectedCB();
					}
				})
			}
		},
		open: function (callback) {
			content.open(function (return_code) {
				callback(return_code.toString());
			});
		},
		close: function (callback) {
			content.close(function (return_code) {
				callback(return_code.toString());
			});
		},
		getState: function (callback) {
			content.getstatus(function (state_info) {
				var state_info_str = state_info.toString().split(",");
				callback(state_info_str[0], state_info_str[1]);
			});
		},
		getInfo: function (callback) {
			content.getinfo(function (info) {
				var info_str = info.toString().split(",");
				var handwareInfoJson = getJsonFromInfo(info_str[0]);
				var deviceStateInfoJson = getJsonFromInfo(info_str[1]);
				var staticInfoJson = getJsonFromInfo(info_str[2]);
				callback(handwareInfoJson, deviceStateInfoJson, staticInfoJson);
			});
		},
		setDefaultOptions: function (options) {
			content.setdefaultoptions(JSON.stringify(options), function (return_code) {
				callback(return_code.toString());
			});
		},
		startCapture:function(callback){
			content.startcapture('{}',function(return_code,job_id){
				callback(return_code.toString(),job_id.toString());
			})
		},
		stopCapture:function(callback){
			content.stopcapture(function(return_code){
				callback(return_code.toString());
			})
		},
		resetDevice:function(callback){
			content.resetdevice(function(return_code){
				callback(return_code.toString());
			})
		},
		facenopassedCompleted:function(callback){
			facenopassedCompletedCB = callback
		},
		facepassCompleted:function(callback){
			facepassCompletedCB = callback
		}
	},
	iocontroler={
		iocontrolerConnectedCB:function () {  },
		connected: function (callback) {
			console.log("connectting")
			iocontrolerConnectedCB = callback;
			var socket = new WebSocket("ws://localhost:12345");
			socket.onclose = function () {
				console.error("web channel close");
			};
			socket.onerror = function (error) {
				console.error("web channel error: " + error);
			}
			socket.onopen = function () {
				console.log("web channel init success.")
				new QWebChannel(socket, function (channel) {
					content = channel.objects.iocontroler;
					if (typeof (iocontrolerConnectedCB) != "undefined") {
						iocontrolerConnectedCB();
					}
				})
			}
		},
		open: function (callback) {
			content.open(function (return_code) {
				callback(return_code.toString());
			});
		},
		close: function (callback) {
			content.close(function (return_code) {
				callback(return_code.toString());
			});
		},
		getState: function (callback) {
			content.getstatus(function (state_info) {
				var state_info_str = state_info.toString().split(",");
				callback(state_info_str[0], state_info_str[1]);
			});
		},
		getInfo: function (callback) {
			content.getinfo(function (info) {
				var info_str = info.toString().split(",");
				var handwareInfoJson = getJsonFromInfo(info_str[0]);
				var deviceStateInfoJson = getJsonFromInfo(info_str[1]);
				var staticInfoJson = getJsonFromInfo(info_str[2]);
				callback(handwareInfoJson, deviceStateInfoJson, staticInfoJson);
			});
		},
		writeGpIo:function(gpio_num,action,callback){
			content.writegpio(gpio_num,action,function(return_code){
				callback(return_code)
			})
		}
	},

    //显示配置
	configdisplay = {
		connected: function (callback) {
			console.log("connectting")
			configdisplayConnectedCB = callback;
			var socket = new WebSocket("ws://localhost:12345");
			socket.onclose = function () {
				console.error("web channel close");
			};
			socket.onerror = function (error) {
				console.error("web channel error: " + error);
			}
			socket.onopen = function () {
				console.log("web channel init success.")
				new QWebChannel(socket, function (channel) {
					configdisplay_content = channel.objects.configdisplay;
					if (typeof (configdisplayConnectedCB) != "undefined") {
						configdisplayConnectedCB();
					}
				})
			}
		},
		Getcrtcsize: function (screen_name,callback) {
			configdisplay_content.getcrtcsize(screen_name,function (return_info) {
				var info_str = return_info.toString().split(",");
				callback(info_str[0],info_str[1]);
			});
		},
		Getscrinfo: function (callback) {
			configdisplay_content.getscrinfo(function (return_info) {
				console.log(return_info)
				var info_str = return_info.toString().split(",");
				console.log(info_str);
				var str_type="";
				for(var i=1;i<info_str.length;i++){
                    			str_type+=info_str[i];
					console.log(str_type);
					str_type+=", ";
				}
				callback(info_str[0],str_type);
			});
		},
		Setscrsize: function (screen_name,res_mode,callback) {
			configdisplay_content.setscrsize(screen_name,res_mode,function (return_code) {
				callback(return_code);
			});
		},
		Setscrrotate: function (screen_name,rotate_dir,callback) {
			configdisplay_content.setscrrotate(screen_name,rotate_dir,function (return_code) {
				callback(return_code);
			});
		},
		Setscrrate: function (screen_name,refresh_rate,callback) {
			configdisplay_content.setscrrate(screen_name,refresh_rate,function (return_code) {
				callback(return_code);
			});
		},
		Setexmonitor: function (home_screen_name,extend_screen_name,relate_pos,callback) {
			configdisplay_content.setexmonitor(home_screen_name,extend_screen_name,relate_pos,function (return_code) {
				console.log(return_code);
				callback(return_code);
			});
		},
		Maptooutput: function (touch_id,touch_screen_name,callback) {
			configdisplay_content.maptooutput(touch_id,touch_screen_name,function (return_code) {
				callback(return_code);
			});
		}
	},
    // 快照设置
	snapshot = {
		connected: function (callback) {
			console.log("connectting")
			snapshotConnectedCB = callback;
			var socket = new WebSocket("ws://localhost:12345");
			socket.onclose = function () {
				console.error("web channel close");
			};
			socket.onerror = function (error) {
				console.error("web channel error: " + error);
			}
			socket.onopen = function () {
				console.log("web channel init success.")
				new QWebChannel(socket, function (channel) {
					snapshot_content = channel.objects.snapshot;
					if (typeof (snapshotConnectedCB) != "undefined") {
						snapshotConnectedCB();
					}
				})
			}
		},
		Openoverlay: function (callback) {
			snapshot_content.openoverlay(function (return_code) {
				callback(return_code);
			});
		},
		Closeoverlay: function (callback) {
			snapshot_content.closeoverlay(function (return_code) {
				callback(return_code);
			});
		},
		Createsnapshot: function (callback) {
			snapshot_content.createsnapshot(function (return_code) {
				callback(return_code);
			});
		},
		Rollbackreset: function (callback) {
			snapshot_content.rollbackreset(function (return_code) {
				callback(return_code);
			});
		},
		Rollbacklast: function (callback) {
			snapshot_content.rollbacklast(function (return_code) {
				callback(return_code);
			});
		}
	},
    //获取核心包列表
	getcorepack = {
		connected: function (callback) {
			console.log("connectting")
			getcorepackConnectedCB = callback;
			var socket = new WebSocket("ws://localhost:12345");
			socket.onclose = function () {
				console.error("web channel close");
			};
			socket.onerror = function (error) {
				console.error("web channel error: " + error);
			}
			socket.onopen = function () {
				console.log("web channel init success.")
				new QWebChannel(socket, function (channel) {
					getcorepack_content = channel.objects.getcorepack;
					if (typeof (getcorepackConnectedCB) != "undefined") {
						getcorepackConnectedCB();
					}
				})
			}
		},
		Getsysver: function (callback) {
			getcorepack_content.getsysver(function (return_info) {
				var info_str = return_info.toString().split(",");
				callback(info_str[0],info_str[1]);
			});
		},
		Getpackinfo: function (callback) {
			getcorepack_content.getpackinfo(function (return_info) {
				console.log(return_info);
				var str_list = new Array();
				for(var i=0;i<return_info[0].length;i++){
                    str_list[0]=return_info[0][i];
					str_list[1]=return_info[1][i];
					callback(str_list[0],str_list[1]);
				}
				
			});
		},
	},
	humanInduction = {
		bodyOnCB:function () {  },
		bodyOffCB:function () {  },
		body:function(a){},
		connected: function (callback) {
			console.log("connectting")
			iocontrolerConnectedCB = callback;
			var socket = new WebSocket("ws://localhost:12345");
			socket.onclose = function () {
				console.error("web channel close");
			};
			socket.onerror = function (error) {
				console.error("web channel error: " + error);
			}
			socket.onopen = function () {
				console.log("web channel init success.")
				new QWebChannel(socket, function (channel) {
					content = channel.objects.humaninduction;
					content.bodyon.connect(function () {
						body(0)
					});
					content.bodyoff.connect(function () {
						body(1)
					});
					if (typeof (iocontrolerConnectedCB) != "undefined") {
						iocontrolerConnectedCB();
					}
				})
			}
		},
		open: function (callback) {
			content.open(function (return_code) {
				callback(return_code.toString());
			});
		},
		close: function (callback) {
			content.close(function (return_code) {
				callback(return_code.toString());
			});
		},
		getState: function (callback) {
			content.getstatus(function (state_info) {
				var state_info_str = state_info.toString().split(",");
				callback(state_info_str[0], state_info_str[1]);
			});
		},
		getInfo: function (callback) {
			content.getinfo(function (info) {
				var info_str = info.toString().split(",");
				var handwareInfoJson = getJsonFromInfo(info_str[0]);
				var deviceStateInfoJson = getJsonFromInfo(info_str[1]);
				var staticInfoJson = getJsonFromInfo(info_str[2]);
				callback(handwareInfoJson, deviceStateInfoJson, staticInfoJson);
			});
		},
		bodyOff:function(callback){
			bodyOffCB = callback
		},
		bodyOn:function (callback) {
			bodyOnCB = callback
		},
		bodyCB:function(callback){
			body=callback
		}
	},
	hidctl = {
			connected: function (callback) {
				console.log("connectting")
				hidctlConnectedCB = callback;
				var socket = new WebSocket("ws://localhost:12345");
				socket.onclose = function () {
					console.error("web channel close");
				};
				socket.onerror = function (error) {
					console.error("web channel error: " + error);
				}
				socket.onopen = function () {
					console.log("web channel init success.")
					new QWebChannel(socket, function (channel) {
						hidcontent = channel.objects.hidctl;
						console.log(hidcontent)
						if (typeof (hidctlConnectedCB) != "undefined") {
							hidctlConnectedCB();
						}
					})
				}
			},
			disable_hid: function (callback) {
				hidcontent.disable_hid(function (return_info) {
					callback(return_info.toString())
				})
			},
			enable_hid: function (callback) {
				hidcontent.enable_hid(function (return_info) {
					callback(return_info.toString())
				})
			},
			get_hid: function (callback) {
				hidcontent.get_hid(function (return_info) {
					callback(return_info.toString())
				})
			}
		},
		readctl = {
			connected: function (callback) {
				console.log("connectting")
				readctlConnectedCB = callback;
				var socket = new WebSocket("ws://localhost:12345");
				socket.onclose = function () {
					console.error("web channel close");
				};
				socket.onerror = function (error) {
					console.error("web channel error: " + error);
				}
				socket.onopen = function () {
					console.log("web channel init success.")
					new QWebChannel(socket, function (channel) {
						readcontent = channel.objects.readctl;
						console.log(readcontent)
						if (typeof (readctlConnectedCB) != "undefined") {
							readctlConnectedCB();
						}
					})
				}
			},
			Open: function (callback) {
				readcontent.Open(function (return_info) {
					callback(return_info.toString())
				})
			},
			Close: function (callback) {
				readcontent.Close(function (return_info) {
					callback(return_info.toString())
				})
			},
			ConnectCard: function (callback) {
				readcontent.ConnectCard(function (return_info) {
					callback(return_info.toString())
				})
			},
			ReadCard: function (callback) {
				readcontent.ReadCard(function (return_info) {
					callback(return_info.toString())
				})
			}
		},
		screenctl = {
			connected: function (callback) {
				console.log("connectting")
				screenctlConnectedCB = callback;
				var socket = new WebSocket("ws://localhost:12345");
				socket.onclose = function () {
					console.error("web channel close");
				};
				socket.onerror = function (error) {
					console.error("web channel error: " + error);
				}
				socket.onopen = function () {
					console.log("web channel init success.")
					new QWebChannel(socket, function (channel) {
						screencontent = channel.objects.screenctl;
						console.log(screencontent)
						if (typeof (screenctlConnectedCB) != "undefined") {
							screenctlConnectedCB();
						}
					})
				}
			},
			Open: function (callback) {
				screencontent.Open(function (return_info) {
					callback(return_info.toString())
				})
			},
			Close: function (callback) {
				screencontent.Close(function (return_info) {
					callback(return_info.toString())
				})
			},
			BeginSign: function (callback) {
				screencontent.BeginSign(function (return_info) {
					callback(return_info.toString())
				})
			},
			EndSign: function (callback) {
				screencontent.EndSign(function (return_info) {
					callback(return_info.toString())
				})
			},
			PrintPicture: function (callback) {
				screencontent.PrintPicture(function (return_info) {
					callback(return_info.toString())
				})
			}
		},
		simreadctl = {
			connected: function (callback) {
				console.log("connectting")
				simreadctlConnectedCB = callback;
				var socket = new WebSocket("ws://localhost:12345");
				socket.onclose = function () {
					console.error("web channel close");
				};
				socket.onerror = function (error) {
					console.error("web channel error: " + error);
				}
				socket.onopen = function () {
					console.log("web channel init success.")
					new QWebChannel(socket, function (channel) {
						simreadcontent = channel.objects.simreadctl;
						console.log(simreadcontent)
						if (typeof (simreadctlConnectedCB) != "undefined") {
							simreadctlConnectedCB();
						}
					})
				}
			},
			Open: function (callback) {
				simreadcontent.Open(function (return_info) {
					callback(return_info.toString())
				})
			},
			Close: function (callback) {
				simreadcontent.Close(function (return_info) {
					callback(return_info.toString())
				})
			},
			PowerOn: function (callback) {
				simreadcontent.PowerOn(function (return_info) {
					callback(return_info)
				})
			},
			PowerOff: function (callback) {
				simreadcontent.PowerOff(function (return_info) {
					callback(return_info)
				})
			},
			SendApud: function (pid, vid, callback) {
				simreadcontent.SendApud(pid, vid, function (return_info) {
					callback(return_info.toString())
				})
			}
		},
		lanreadctl = {
			connected: function (callback) {
				console.log("connectting")
				lanreadctlConnectedCB = callback;
				var socket = new WebSocket("ws://localhost:12345");
				socket.onclose = function () {
					console.error("web channel close");
				};
				socket.onerror = function (error) {
					console.error("web channel error: " + error);
				}
				socket.onopen = function () {
					console.log("web channel init success.")
					new QWebChannel(socket, function (channel) {
						lanreadcontent = channel.objects.lanreadctl;
						console.log(readcontent)
						if (typeof (lanreadctlConnectedCB) != "undefined") {
							lanreadctlConnectedCB();
						}
					})
				}
			},
			ReadCard: function (callback) {
				lanreadcontent.ReadCard(function (return_info) {
					callback(return_info.toString())
				})
			}
		},
		userctl = {
			connected: function (callback) {
				console.log("connectting")
				userctlConnectedCB = callback;
				var socket = new WebSocket("ws://localhost:12345");
				socket.onclose = function () {
					console.error("web channel close");
				};
				socket.onerror = function (error) {
					console.error("web channel error: " + error);
				}
				socket.onopen = function () {
					console.log("web channel init success.")
					new QWebChannel(socket, function (channel) {
						usercontent = channel.objects.userctl;
						console.log(usercontent)
						if (typeof (userctlConnectedCB) != "undefined") {
							userctlConnectedCB();
						}
					})
				}
			},
			getusername: function (callback) {
				usercontent.getusername(function (return_info) {
					callback(return_info.toString())
				})
			}
		},
		videoctl = {
			connected: function (callback) {
				console.log("connectting")
				videoctlConnectedCB = callback;
				var socket = new WebSocket("ws://localhost:12345");
				socket.onclose = function () {
					console.error("web channel close");
				};
				socket.onerror = function (error) {
					console.error("web channel error: " + error);
				}
				socket.onopen = function () {
					console.log("web channel init success.")
					new QWebChannel(socket, function (channel) {
						videocontent = channel.objects.videoctl;
						console.log(videocontent)
						if (typeof (videoctlConnectedCB) != "undefined") {
							videoctlConnectedCB();
						}
					})
				}
			},
			disable_video: function (callback) {
				videocontent.disable_video(function (return_info) {
					callback(return_info.toString())
				})
			},
			enable_video: function (callback) {
				videocontent.enable_video(function (return_info) {
					callback(return_info.toString())
				})
			},
			get_video: function (callback) {
				videocontent.get_video(function (return_info) {
					callback(return_info.toString())
				})
			}
		},
		storagectl = {
			connected: function (callback) {
				console.log("connectting")
				storagectlConnectedCB = callback;
				var socket = new WebSocket("ws://localhost:12345");
				socket.onclose = function () {
					console.error("web channel close");
				};
				socket.onerror = function (error) {
					console.error("web channel error: " + error);
				}
				socket.onopen = function () {
					console.log("web channel init success.")
					new QWebChannel(socket, function (channel) {
						storagecontent = channel.objects.storagectl;
						console.log(storagecontent)
						if (typeof (storagectlConnectedCB) != "undefined") {
							storagectlConnectedCB();
						}
					})
				}
			},
			set_status: function (mode, callback) {
				storagecontent.set_status(mode, function (return_info) {
					callback(return_info.toString())
				})
			},
			get_status: function (callback) {
				storagecontent.get_status(function (return_info) {
					callback(return_info.toString())
				})
			}
		},
		listctl = {
			connected: function (callback) {
				console.log("connectting")
				listctlConnectedCB = callback;
				var socket = new WebSocket("ws://localhost:12345");
				socket.onclose = function () {
					console.error("web channel close");
				};
				socket.onerror = function (error) {
					console.error("web channel error: " + error);
				}
				socket.onopen = function () {
					console.log("web channel init success.")
					new QWebChannel(socket, function (channel) {
						listontent = channel.objects.listctl;
						console.log(listontent)
						if (typeof (listctlConnectedCB) != "undefined") {
							listctlConnectedCB();
						}
					})
				}
			},
			SetWhiteList: function (pid, vid, callback) {
				listontent.SetWhiteList(pid, vid, function (return_info) {
					callback(return_info.toString())
				})
			},
			GetWhiteList: function (callback) {
				listontent.GetWhiteList(function (return_info) {
					callback(return_info.toString())
				})
			},

			ClearWhiteList: function (callback) {
				listontent.ClearWhiteList(function (return_info) {
					callback(return_info.toString())
				})
			},
			SetBlackList: function (pid, vid, callback) {
				listontent.SetBlackList(pid, vid, function (return_info) {
					callback(return_info.toString())
				})
			},
			GetBlackList: function (callback) {
				listontent.GetBlackList(function (return_info) {
					callback(return_info.toString())
				})
			},
			ClearBlackList: function (callback) {
				listontent.ClearBlackList(function (return_info) {
					callback(return_info.toString())
				})
			}
		},

		bluez = {
			connected: function (callback) {
				console.log("connectting")
				bluezConnectedCB = callback;
				var socket = new WebSocket("ws://localhost:12345");
				socket.onclose = function () {
					console.error("web channel close");
				};
				socket.onerror = function (error) {
					console.error("web channel error: " + error);
				}
				socket.onopen = function () {
					console.log("web channel init success.")
					new QWebChannel(socket, function (channel) {
						bluezcontent = channel.objects.bluez;
						if (typeof (bluezConnectedCB) != "undefined") {
							bluezConnectedCB();
						}
					})
				}
			},

			RegisterAgent: function (obj_path, capability, callback) {
				bluezcontent.RegisterAgent(obj_path, capability, function (return_code) {
					callback(return_code.toString());
				});
			},
			RequestDefaultAgent: function (obj_path, callback) {
				bluezcontent.RequestDefaultAgent(obj_path, function (return_code) {
					callback(return_code.toString());
				});
			},
			UnregisterAgent: function (obj_path, callback) {
				bluezcontent.UnregisterAgent(obj_path, function (return_code) {
					callback(return_code.toString());
				})
			}

		},

		bluezhci = {
			connected: function (callback) {
				console.log("connectting")
				bluezhciConnectedCB = callback;
				var socket = new WebSocket("ws://localhost:12345");
				socket.onclose = function () {
					console.error("web channel close");
				};
				socket.onerror = function (error) {
					console.error("web channel error: " + error);
				}
				socket.onopen = function () {
					console.log("web channel init success.")
					new QWebChannel(socket, function (channel) {
						bluezhcicontent = channel.objects.bluezhci;
						if (typeof (bluezhciConnectedCB) != "undefined") {
							bluezhciConnectedCB();
						}
					})
				}
			},

			GetDiscoveryFilters: function (callback) {
				bluezhcicontent.GetDiscoveryFilters(function (return_code) {
					callback(return_code.toString());
				});
			},
			RemoveDevice: function (obj_path, callback) {
				bluezhcicontent.RemoveDevice(obj_path, function (return_code) {
					callback(return_code.toString());
				});
			},
			SetDiscoveryFilter: function (properties, callback) {
				bluezhcicontent.SetDiscoveryFilter(properties, function (return_code) {
					callback(return_code.toString());
				});
			},
			StartDiscovery: function (callback) {
				bluezhcicontent.StartDiscovery(function (return_code) {
					callback(return_code.toString());
				})
			},
			StopDiscovery: function (callback) {
				bluezhcicontent.StopDiscovery(function (return_code) {
					callback(return_code.toString());
				});
			}
		},

		bluezobex = {
			connected: function (callback) {
				console.log("connectting")
				bluezobexConnectedCB = callback;
				var socket = new WebSocket("ws://localhost:12345");
				socket.onclose = function () {
					console.error("web channel close");
				};
				socket.onerror = function (error) {
					console.error("web channel error: " + error);
				}
				socket.onopen = function () {
					console.log("web channel init success.")
					new QWebChannel(socket, function (channel) {
						bluezobexcontent = channel.objects.bluezobex;
						if (typeof (bluezobexConnectedCB) != "undefined") {
							bluezobexConnectedCB();
						}
					})
				}
			},

			CreateSession: function (destination, args, callback) {
				bluezobexcontent.CreateSession(destination, JSON.stringify(args), function (return_code) {
					console.log(args);
					callback(return_code.toString());
				});
			},
			RemoveSession: function (obj_path, callback) {
				bluezobexcontent.RemoveSession(obj_path, function (return_code) {
					callback(return_code.toString());
				});
			},
		},
		bluezobexclient = {
			connected: function (callback) {
				console.log("connectting")
				bluezobexclientConnectedCB = callback;
				var socket = new WebSocket("ws://localhost:12345");
				socket.onclose = function () {
					console.error("web channel close");
				};
				socket.onerror = function (error) {
					console.error("web channel error: " + error);
				}
				socket.onopen = function () {
					console.log("web channel init success.")
					new QWebChannel(socket, function (channel) {
						bluezobexclientcontent = channel.objects.bluezobexclient;
						if (typeof (bluezobexclientConnectedCB) != "undefined") {
							bluezobexclientConnectedCB();
						}
					})
				}
			},

			ExchangeBusinessCards: function (clientfile, targetfile, callback) {
				bluezobexclientcontent.ExchangeBusinessCards(clientfile, targetfile, function (return_code) {
					callback(return_code.toString());
				})
			},
			PullBusinessCard: function (targetfile, callback) {
				bluezobexclientcontent.PullBusinessCard(targetfile, function (return_code) {
					callback(return_code.toString());
				});
			},
			SendFile: function (sourcefile, callback) {
				bluezobexclientcontent.SendFile(sourcefile, function (return_code) {
					callback(return_code.toString());
				});
			}

		},
		bluezobjectmonitor = {
			connected: function (callback) {
				console.log("connectting")
				bluezobjectmonitorConnectedCB = callback;
				var socket = new WebSocket("ws://localhost:12345");
				socket.onclose = function () {
					console.error("web channel close");
				};
				socket.onerror = function (error) {
					console.error("web channel error: " + error);
				}
				socket.onopen = function () {
					console.log("web channel init success.")
					new QWebChannel(socket, function (channel) {
						bluezobjectmonitorcontent = channel.objects.bluezobjectmonitor;
						console.log(bluezobjectmonitorcontent)
						if (typeof (bluezobjectmonitorConnectedCB) != "undefined") {
							bluezobjectmonitorConnectedCB();
						}
					})
				}
			},
			GetManagedObject: function (seach_time, callback) {
				bluezobjectmonitorcontent.GetManagedObject(seach_time, function (return_code) {
					callback(return_code);
				});
			},

			GetObjectPath: function (bluez_obj_path, callback) {
				bluezobjectmonitorcontent.GetObjectPath(bluez_obj_path, function (return_code) {
					callback(return_code.toString());
				})
			},
			ConnectBlueDev: function (bluez_obj_path, bluez_create_path, callback) {
				bluezobjectmonitorcontent.ConnectBlueDev(bluez_obj_path, bluez_create_path, function (return_code) {
					callback(return_code);
				})
			},
			DisConnect: function (callback) {
				bluezobjectmonitorcontent.DisConnect(function (return_code) {
					callback(return_code.toString());
				})
			},

		},
		network = {
			connected: function (callback) {
				console.log("connectting")
				networkConnectedCB = callback;
				var socket = new WebSocket("ws://localhost:12345");
				socket.onclose = function () {
					console.error("web channel close");
				};
				socket.onerror = function (error) {
					console.error("web channel error: " + error);
				}
				socket.onopen = function () {
					console.log("web channel init success.")
					new QWebChannel(socket, function (channel) {
						networkcontent = channel.objects.network;
						if (typeof (networkConnectedCB) != "undefined") {
							networkConnectedCB();
						}
					})
				}
			},

			Reload: function (flags, callback) {
				networkcontent.Reload(flags, function (return_code) {
					callback(return_code.toString());
				})
			},
			DeactivateConnection: function (active_connection, callback) {
				networkcontent.DeactivateConnection(active_connection, function (return_code) {
					callback(return_code.toString());
				})
			},
			Sleep: function (sleep, callback) {
				networkcontent.Sleep(sleep, function (return_code) {
					callback(return_code.toString());
				})
			},
			Enable: function (enable, callback) {
				networkcontent.Enable(enable, function (return_code) {
					callback(return_code.toString());
				})
			},
			SetLogging: function (level, domains, callback) {
				networkcontent.SetLogging(level, domains, function (return_code) {
					callback(return_code.toString());
				})
			},
			GetDevices: function (callback) {
				networkcontent.GetDevices(function (return_code) {
					callback(return_code.toString());
				})
			},
			GetAllDevices: function (callback) {
				networkcontent.GetAllDevices(function (return_code) {
					callback(return_code);
				})
			},
			GetDeviceByIpIface: function (iface, callback) {
				networkcontent.GetDeviceByIpIface(iface, function (return_code) {
					callback(return_code.toString());
				})
			},
			ActivateConnection: function (connection, device, speciflc_object, callback) {
				networkcontent.ActivateConnection(connection, device, speciflc_object, function (return_code) {
					callback(return_code.toString());
				})
			},
			AddAndActivateConnection: function (connection, device, speciflc_object, callback) {
				networkcontent.AddAndActivateConnection(connection, device, speciflc_object, function (return_code) {
					callback(return_code.toString());
				})
			},
			AddAndetherConnection: function (device, ip, dns, prefix, geteway, callback) {
				networkcontent.AddAndetherConnection(device, ip, dns, prefix, geteway, function (return_code) {
					callback(return_code.toString());
				})
			},
			GetPermissions: function (callback) {
				networkcontent.GetPermissions(function (return_code) {
					callback(return_code.toString());
				})
			},
			GetLogging: function (callback) {
				networkcontent.GetLogging(function (return_code) {
					callback(return_code.toString());
				})
			},
			CheckConnectivity: function (callback) {
				networkcontent.CheckConnectivity(function (return_code) {
					callback(return_code.toString());
				})
			},
			state: function (callback) {
				networkcontent.state(function (return_code) {
					callback(return_code.toString());
				})
			},
			enable_wifi: function (callback) {
				networkcontent.enable_wifi(function (return_code) {
					callback(return_code.toString());
				})
			},
		},
		networkobject = {
			connected: function (callback) {
				console.log("connectting")
				networkobjectConnectedCB = callback;
				var socket = new WebSocket("ws://localhost:12345");
				socket.onclose = function () {
					console.error("web channel close");
				};
				socket.onerror = function (error) {
					console.error("web channel error: " + error);
				}
				socket.onopen = function () {
					console.log("web channel init success.")
					new QWebChannel(socket, function (channel) {
						networkobjectcontent = channel.objects.networkobject;
						if (typeof (networkobjectConnectedCB) != "undefined") {
							networkobjectConnectedCB();
						}
					})
				}
			},
			GetManagedObjects: function (callback) {
				networkobjectcontent.GetManagedObjects(function (return_code) {
					callback(return_code.toString());
				})
			},
		},
		bluedev = {
			PropertiesChanged: function () { },
			connected: function (callback) {
				console.log("bluedev")
				bluedevConnectedCB = callback;
				var socket = new WebSocket("ws://localhost:12345");
				socket.onclose = function () {
					console.error("web channel close");
				};
				socket.onerror = function (error) {
					console.error("web channel error: " + error);
				}
				socket.onopen = function () {
					console.log("web channel init success.")
					new QWebChannel(socket, function (channel) {
						bluedevcontent = channel.objects.bluedev;
						console.log(bluedevcontent)
						bluedevcontent.PropertiesChanged.connect(function (data) {
							console.log(data)
							PropertiesChanged(data)
						})
						if (typeof (bluedevConnectedCB) != "undefined") {
							bluedevConnectedCB();
						}
					})
				}
			}
		},
		bluezsendstatu = {
			connected: function (callback) {
				console.log("connectting")
				bluezsendstatuConnectedCB = callback;
				var socket = new WebSocket("ws://localhost:12345");
				socket.onclose = function () {
					console.error("web channel close");
				};
				socket.onerror = function (error) {
					console.error("web channel error: " + error);
				}
				socket.onopen = function () {
					console.log("web channel init success.")
					new QWebChannel(socket, function (channel) {
						bluezsendstatucontent = channel.objects.bluezsendstatu;
						if (typeof (bluezsendstatuConnectedCB) != "undefined") {
							bluezsendstatuConnectedCB();
						}
					})
				}
			},
			GetStatu: function (callback) {
				bluezsendstatucontent.GetStatu(function (return_code) {
					callback(return_code.toString());
				})
			}
		},
		bluezsetup = {
			connected: function (callback) {
				console.log("connectting")
				bluezsetupConnectedCB = callback;
				var socket = new WebSocket("ws://localhost:12345");
				socket.onclose = function () {
					console.error("web channel close");
				};
				socket.onerror = function (error) {
					console.error("web channel error: " + error);
				}
				socket.onopen = function () {
					console.log("web channel init success.")
					new QWebChannel(socket, function (channel) {
						bluezsetupcontent = channel.objects.bluezsetup;
						if (typeof (bluezsetupConnectedCB) != "undefined") {
							bluezsetupConnectedCB();
						}
					})
				}
			},
			bluez_set_up: function (callback) {
				bluezsetupcontent.bluez_set_up(function (return_code) {
					callback(return_code.toString());
				})
			}
		},
		bluezlist = {
			connected: function (callback) {
				console.log("connectting")
				bluezlistConnectedCB = callback;
				var socket = new WebSocket("ws://localhost:12345");
				socket.onclose = function () {
					console.error("web channel close");
				};
				socket.onerror = function (error) {
					console.error("web channel error: " + error);
				}
				socket.onopen = function () {
					console.log("web channel init success.")
					new QWebChannel(socket, function (channel) {
						bluezlistcontent = channel.objects.bluezlist;
						if (typeof (bluezlistConnectedCB) != "undefined") {
							bluezlistConnectedCB();
						}
					})
				}
			},
			add_list: function (mode, device_mac, callback) {
				bluezlistcontent.add_list(mode, device_mac, function (return_code) {
					callback(return_code.toString());
				})
			},

			set_active_list: function (mode, callback) {
				bluezlistcontent.set_active_list(mode, function (return_code) {
					callback(return_code.toString());
				})
			},
			get_active_list: function (callback) {
				bluezlistcontent.get_active_list(function (return_code) {
					callback(return_code.toString());
				})
			},
			clear_list: function (mode, callback) {
				bluezlistcontent.clear_list(mode, function (return_code) {
					callback(return_code.toString());
				})
			},
			get_list: function (mode, callback) {
				bluezlistcontent.get_list(mode, function (return_code) {
					callback(return_code);
				})
			}
		},
		networkwifi = {
			connected: function (callback) {
				console.log("connectting")
				networkwifiConnectedCB = callback;
				var socket = new WebSocket("ws://localhost:12345");
				socket.onclose = function () {
					console.error("web channel close");
				};
				socket.onerror = function (error) {
					console.error("web channel error: " + error);
				}
				socket.onopen = function () {
					console.log("web channel init success.")
					new QWebChannel(socket, function (channel) {
						networkwificontent = channel.objects.networkwifi;
						if (typeof (networkwifiConnectedCB) != "undefined") {
							networkwifiConnectedCB();
						}
					})
				}
			},
			ScanWifi: function (callback) {
				networkwificontent.ScanWifi(function (return_code) {
					callback(return_code);
				})
			}
		},
		networksettings = {
			connected: function (callback) {
				console.log("connectting")
				networksettingsConnectedCB = callback;
				var socket = new WebSocket("ws://localhost:12345");
				socket.onclose = function () {
					console.error("web channel close");
				};
				socket.onerror = function (error) {
					console.error("web channel error: " + error);
				}
				socket.onopen = function () {
					console.log("web channel init success.")
					new QWebChannel(socket, function (channel) {
						networksettingscontent = channel.objects.networksettings;
						if (typeof (networksettingsConnectedCB) != "undefined") {
							networksettingsConnectedCB();
						}
					})
				}
			},
			AddWifiConnection: function (access_point_path, password, callback) {
				networksettingscontent.AddWifiConnection(access_point_path, password, function (return_code) {
					callback(return_code);
				})
			},
			hostpot: function (host_name, password, ip, dns, prefix, geteway, callback) {
				networksettingscontent.hostpot(host_name, password, ip, dns, prefix, geteway, function (return_code) {
					callback(return_code);
				})
			},
			Disconnect: function (callback) {
				networksettingscontent.Disconnect(function (return_code) {
					callback(return_code);
				})
			}
		},
		wifilist = {
			connected: function (callback) {
				console.log("connectting")
				wifilistConnectedCB = callback;
				var socket = new WebSocket("ws://localhost:12345");
				socket.onclose = function () {
					console.error("web channel close");
				};
				socket.onerror = function (error) {
					console.error("web channel error: " + error);
				}
				socket.onopen = function () {
					console.log("web channel init success.")
					new QWebChannel(socket, function (channel) {
						wifilistcontent = channel.objects.wifilist;
						if (typeof (wifilistConnectedCB) != "undefined") {
							wifilistConnectedCB();
						}
					})
				}
			},
			add_list: function (mode, device_mac, callback) {
				wifilistcontent.add_list(mode, device_mac, function (return_code) {
					callback(return_code.toString());
				})
			},

			set_active_list: function (mode, callback) {
				wifilistcontent.set_active_list(mode, function (return_code) {
					callback(return_code.toString());
				})
			},
			get_active_list: function (callback) {
				wifilistcontent.get_active_list(function (return_code) {
					callback(return_code.toString());
				})
			},
			clear_list: function (mode, callback) {
				wifilistcontent.clear_list(mode, function (return_code) {
					callback(return_code.toString());
				})
			},
			get_list: function (mode, callback) {
				wifilistcontent.get_list(mode, function (return_code) {
					callback(return_code);
				})
			}
		},
		usbctl = {
			connected: function (callback) {
				console.log("connectting")
				usbctlConnectedCB = callback;
				var socket = new WebSocket("ws://localhost:12345");
				socket.onclose = function () {
					console.error("web channel close");
				};
				socket.onerror = function (error) {
					console.error("web channel error: " + error);
				}
				socket.onopen = function () {
					console.log("web channel init success.")
					new QWebChannel(socket, function (channel) {
						usbctlcontent = channel.objects.usbctl;
						if (typeof (usbctlConnectedCB) != "undefined") {
							usbctlConnectedCB();
						}
					})
				}
			},
			USB_forbid: function (callback) {
				usbctlcontent.USB_forbid(function (return_code) {
					callback(return_code.toString());
				})
			},

			USB_allow: function (callback) {
				usbctlcontent.USB_allow(function (return_code) {
					callback(return_code.toString());
				})
			}
		},
		ukey = {
			ukeyjobcompletedCB:function(){},
			connected: function (callback) {
				console.log("connectting")
				ukeyConnectedCB = callback;
				var socket = new WebSocket("ws://localhost:12345");
				socket.onclose = function () {
					console.error("web channel close");
				};
				socket.onerror = function (error) {
					console.error("web channel error: " + error);
				}
				socket.onopen = function () {
					console.log("web channel init success.")
					new QWebChannel(socket, function (channel) {
						ukeycontent = channel.objects.ukey;
						console.log(ukeycontent)
						ukeycontent.ukeyjobcompleted.connect(function(first, second,thirds)
						{
							console.log(first, second,thirds)
							ukeyjobcompletedCB(first, second,thirds)
						})
						if (typeof (ukeyConnectedCB) != "undefined") {
							ukeyConnectedCB();
						}
					})
				}
			},
			initpin:function(pincode,callback){
				ukeycontent.initpin(pincode,function(return_info){
					callback(return_info.toString())
				})
			},
			delfile:function(ukeyhandle, callback){
				ukeycontent.delfile(ukeyhandle, function(return_info){
					callback(return_info.toString())
				})
			},
			genrsakey:function(keylen,callback){
				ukeycontent.genrsakey(keylen,function(return_info){
					callback(return_info.toString())
				})
			},
			gencsr:function(csr_path,callback){
				ukeycontent.gencsr(csr_path,function(return_info){
					callback(return_info)
				})
			},
			localgencsr:function(csr_path, key_path, callback){
				ukeycontent.localgencsr(csr_path, key_path, function(return_info){
					callback(return_info)
				})
			},
			encrypt:function(data,callback){
				ukeycontent.encrypt(data,function(return_info){
					callback(return_info.toString())
				})
			},
			decrypt:function(data, callback){
				ukeycontent.decrypt(data, function(return_info){
					callback(return_info.toString())
				})
			},
			exportcert:function(cert_path,callback){
				ukeycontent.exportcert(cert_path,function(return_info){
					callback(return_info.toString())
				})
			},
			readfile:function(len, filename, callback){
				ukeycontent.readfile(len, filename, function(return_info){
					callback(return_info)
				})
			},
			import_cert:function(path, callback){
				ukeycontent.import_cert(path, function(return_info){
					callback(return_info)
				})
			},
			changepin:function(newpin, oldpin, callback){
				ukeycontent.changepin(newpin, oldpin, function(return_info){
					callback(return_info)
				})
			},
			writefile:function(buffer, buflen, handle, callback){
				ukeycontent.writefile(buffer, buflen, handle, function(return_info){
					callback(return_info)
				})
			},
			importfile:function(path, filename, size, callback){
				ukeycontent.importfile(path, filename, size,function(return_info){
					callback(return_info)
				})
			},
			exportfile:function(path, filename, size, callback){
				ukeycontent.exportfile(path, filename, size, function(return_info){
					callback(return_info)
				})
			},
			open:function(callback){
				ukeycontent.open(function(return_info){
					callback(return_info)
				})
			},
			close:function(callback){
				ukeycontent.close(function(return_info){
					callback(return_info)
				})
			},
			ukeyjobcompleted:function(callback){
				ukeyjobcompletedCB=callback
			}
		}
// powercontrol.connected()
bluetoothle.connected()
snapshot.connected()
reboothostapd.connected()
}
