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
output=function(message){
	var output = document.getElementById("StateResult");
	output.innerHTML = output.innerHTML + message + "\n";
},
getJsonFromInfo=function(info){
	if(info == ""){
		return "{}";
	}
	var info_list = info.split("&");
	var info_json = "{";
	for(var i=0; i<info_list.length;i++){
		var key_value_list = info_list[i].split(":");
		var key = key_value_list[0];
		var value = key_value_list[1];
		if(i==0){
			info_json += "\"" + key + "\"" + ":" + "\"" + value + "\"";
		}else{
			info_json += ",\"" + key + "\"" + ":" + "\"" + value + "\"";
		}
	}
	info_json += "}";
	return info_json;	
},
isEmptyObject=function(obj){
	for (var key in obj) {
		return false;
	}
	return true;
},
configmanager={
	cmConnectedCB:function(){},
	init:function(){
		var socket = new WebSocket("ws://localhost:12345");
		socket.onopen = function(){
			//output("socket opened");
			new QWebChannel(socket,function(channel){
				cmobject = channel.objects.configmanager;
				if(typeof(cmConnectedCB) != "undefined"){
					cmConnectedCB();
				}
			});
		};
	},
	connected:function(callback){
		if(typeof(cmobject) != "undefined"){
			callback();
		}else{
			cmConnectedCB=callback;	
		}
	},
	getConf:function(sectionname,callback){
		cmobject.getconf(sectionname,callback);	
	},
	setConf:function(sectionname,data,callback){
		//cmobject.setconf(sectionname,JSON.stringify(data),callback);
		cmobject.setconf(sectionname,data,function(result){
            callback(result.toString());
        });
	},
    reboot:function(isoverlay,callback){
        cmobject.reboot(isoverlay,callback);
    }
},
confpage={
	getForm:function(){},
	confpageForm:function(callback){
		console.log('CALLTRACE : confpageForm Is Running.');
		getForm=callback;
	},
	connected:function(callback){

	var socket = new WebSocket("ws://localhost:12345");
	callback();
    socket.onclose = function()
    {
        console.error("web channel closed");
    };
    socket.onerror = function(error)
    {
        console.error("web channel error: " + error);
    };
    socket.onopen = function()
    {
    	console.log("web channel init success.");
    	
        new QWebChannel(socket, function(channel)
        {
            content =channel.objects.confpage;
            // console.log('-++++++-')

            content.ConfpageForm.connect(function(form){

					// console.log('1111111111111111');
					console.log(form);
					getForm(form);
					// console.log('2222222222');
					})
            console.log('---------------');
        });
    };
	},
	commitForm:function(form){
		content.HandleConfPage(form)
	}
}
}
// confpage.init();
configmanager.init();
