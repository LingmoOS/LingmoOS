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

var serviceT = {};

serviceT.ListCard = function(){
	var resultInfo;
	var time = new Date();
	var tes = "http://127.0.0.1:19002/api/device/ListCard?time="+time;
	jQuery.support.cors = true;
	$.ajax({  
		type : "post",  
		async:false,  
		url : tes,
		jsonp: "jsonpCallback",//服务端用于接收callback调用的function名的参数  
		success : function(result){
			if(result.indexOf("jsonpCallback")!=-1){
				resultInfo = result.substring(14,result.length-1);
			}else{
				resultInfo = result;
			}	
		},  
		error:function(){  
			resultInfo = "{\"Result\":\"-1\",\"ResultMessage\":\"网络问题,请确认服务是否存在\"}";
		}
	})
	return resultInfo;
}

serviceT.getSysLegalResolution = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8090/resolution/getSysLegalResolution";
	jQuery.support.cors = true;
	$.ajax({  
		type : "get",  
		async:false,  
		url : tes,
		success : function(result){
			if(result!=-1){
				// resultInfo = result.substring(14,result.length-1);
                var jsonData = JSON.stringify(result);
				resultInfo = jsonData;
			}else{
				resultInfo = result;
			}	
		},  
		error:function(){  
			resultInfo = "{\"Result\":\"-1\",\"ResultMessage\":\"网络问题,请确认服务是否存在\"}";
		}
	})
	return resultInfo;
}
serviceT.getNetState = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8090/netLink/getNetState";
	jQuery.support.cors = true;
	$.ajax({  
		type : "get",  
		async:false,  
		url : tes,
		success : function(result){
			if(result!=-1){
				// resultInfo = result.substring(14,result.length-1);
                var jsonData = JSON.stringify(result);
				resultInfo = jsonData;
			}else{
				resultInfo = result;
			}	
		},  
		error:function(){  
			resultInfo = "{\"Result\":\"-1\",\"ResultMessage\":\"网络问题,请确认服务是否存在\"}";
		}
	})
	return resultInfo;
}
serviceT.getAllUsbInfo = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8090/peripheralsenum/getAllUsbInfo";
	jQuery.support.cors = true;
	$.ajax({  
		type : "get",  
		async:false,  
		url : tes,
		success : function(result){
			if(result!=-1){
				// resultInfo = result.substring(14,result.length-1);
                var jsonData = JSON.stringify(result);
				resultInfo = jsonData;
			}else{
				resultInfo = result;
			}	
		},  
		error:function(){  
			resultInfo = "{\"Result\":\"-1\",\"ResultMessage\":\"网络问题,请确认服务是否存在\"}";
		}
	})
	return resultInfo;
}
serviceT.getNetCardName = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8090/netcard/getNetCardName";
	jQuery.support.cors = true;
	$.ajax({  
		type : "get",  
		async:false,  
		url : tes,
		success : function(result){
			if(result!=-1){
				// resultInfo = result.substring(14,result.length-1);
                var jsonData = JSON.stringify(result);
				resultInfo = jsonData;
			}else{
				resultInfo = result;
			}	
		},  
		error:function(){  
			resultInfo = "{\"Result\":\"-1\",\"ResultMessage\":\"网络问题,请确认服务是否存在\"}";
		}
	})
	return resultInfo;
}
serviceT.getNetCardType = function(pid){
	var resultInfo;
	var tes = "http://127.0.0.1:8090/netcard/getNetCardType?netCardName="+pid;
    console.log(tes)
	jQuery.support.cors = true;
	$.ajax({  
		type : "get",  
		async:false,  
		url : tes,
		success : function(result){
			if(result!=-1){
				// resultInfo = result.substring(14,result.length-1);
                var jsonData = JSON.stringify(result);
				resultInfo = jsonData;
			}else{
				resultInfo = result;
			}	
		},  
		error:function(){  
			resultInfo = "{\"Result\":\"-1\",\"ResultMessage\":\"网络问题,请确认服务是否存在\"}";
		}
	})
	return resultInfo;
}
serviceT.getNetCardProduct = function(pid){
	var resultInfo;
	var tes = "http://127.0.0.1:8090/netcard/getNetCardProduct?netCardName="+pid;
    console.log(tes)
	jQuery.support.cors = true;
	$.ajax({  
		type : "get",  
		async:false,  
		url : tes,
		success : function(result){
			if(result!=-1){
				// resultInfo = result.substring(14,result.length-1);
                var jsonData = JSON.stringify(result);
				resultInfo = jsonData;
			}else{
				resultInfo = result;
			}	
		},  
		error:function(){  
			resultInfo = "{\"Result\":\"-1\",\"ResultMessage\":\"网络问题,请确认服务是否存在\"}";
		}
	})
	return resultInfo;
}
serviceT.getBiosVendorVersion = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8090/bios/getBiosVendorVersion";
	jQuery.support.cors = true;
	$.ajax({  
		type : "get",  
		async:false,  
		url : tes,
		success : function(result){
			if(result!=-1){
                var jsonData = JSON.stringify(result);
				resultInfo = jsonData;
			}else{
				resultInfo = result;
			}	
		},  
		error:function(){  
			resultInfo = "{\"Result\":\"-1\",\"ResultMessage\":\"网络问题,请确认服务是否存在\"}";
		}
	})
	return resultInfo;
}
serviceT.getMainboardName = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8090/mainboard/getMainboardName";
	jQuery.support.cors = true;
	$.ajax({  
		type : "get",  
		async:false,  
		url : tes,
		success : function(result){
			if(result!=-1){
                var jsonData = JSON.stringify(result);
				resultInfo = jsonData;
			}else{
				resultInfo = result;
			}	
		},  
		error:function(){  
			resultInfo = "{\"Result\":\"-1\",\"ResultMessage\":\"网络问题,请确认服务是否存在\"}";
		}
	})
	return resultInfo;
}
serviceT.getMainboardDate = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8090/mainboard/getMainboardDate";
	jQuery.support.cors = true;
	$.ajax({  
		type : "get",  
		async:false,  
		url : tes,
		success : function(result){
			if(result!=-1){
                var jsonData = JSON.stringify(result);
				resultInfo = jsonData;
			}else{
				resultInfo = result;
			}	
		},  
		error:function(){  
			resultInfo = "{\"Result\":\"-1\",\"ResultMessage\":\"网络问题,请确认服务是否存在\"}";
		}
	})
	return resultInfo;
}
serviceT.getMainboardSerial = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8090/mainboard/getMainboardSerial";
	jQuery.support.cors = true;
	$.ajax({  
		type : "get",  
		async:false,  
		url : tes,
		success : function(result){
			if(result!=-1){
                var jsonData = JSON.stringify(result);
				resultInfo = jsonData;
			}else{
				resultInfo = result;
			}	
		},  
		error:function(){  
			resultInfo = "{\"Result\":\"-1\",\"ResultMessage\":\"网络问题,请确认服务是否存在\"}";
		}
	})
	return resultInfo;
}
serviceT.getMainboardVendor = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8090/mainboard/getMainboardVendor";
	jQuery.support.cors = true;
	$.ajax({  
		type : "get",  
		async:false,  
		url : tes,
		success : function(result){
			if(result!=-1){
                var jsonData = JSON.stringify(result);
				resultInfo = jsonData;
			}else{
				resultInfo = result;
			}	
		},  
		error:function(){  
			resultInfo = "{\"Result\":\"-1\",\"ResultMessage\":\"网络问题,请确认服务是否存在\"}";
		}
	})
	return resultInfo;
}
serviceT.getDiskType = function(pid){
	var resultInfo;
	var tes = "http://127.0.0.1:8090/disk/getDiskType?diskname="+pid;
    console.log(tes)
	jQuery.support.cors = true;
	$.ajax({  
		type : "get",  
		async:false,  
		url : tes,
		success : function(result){
			if(result!=-1){
				// resultInfo = result.substring(14,result.length-1);
                var jsonData = JSON.stringify(result);
				resultInfo = jsonData;
			}else{
				resultInfo = result;
			}	
		},  
		error:function(){  
			resultInfo = "{\"Result\":\"-1\",\"ResultMessage\":\"网络问题,请确认服务是否存在\"}";
		}
	})
	return resultInfo;
}
serviceT.getDiskVersion = function(pid){
	var resultInfo;
	var tes = "http://127.0.0.1:8090/disk/getDiskVersion?diskname="+pid;
    console.log(tes)
	jQuery.support.cors = true;
	$.ajax({  
		type : "get",  
		async:false,  
		url : tes,
		success : function(result){
			if(result!=-1){
				// resultInfo = result.substring(14,result.length-1);
                var jsonData = JSON.stringify(result);
				resultInfo = jsonData;
			}else{
				resultInfo = result;
			}	
		},  
		error:function(){  
			resultInfo = "{\"Result\":\"-1\",\"ResultMessage\":\"网络问题,请确认服务是否存在\"}";
		}
	})
	return resultInfo;
}
serviceT.getDiskSpeed = function(pid){
	var resultInfo;
	var tes = "http://127.0.0.1:8090/disk/getDiskSpeed?diskname="+pid;
    console.log(tes)
	jQuery.support.cors = true;
	$.ajax({  
		type : "get",  
		async:false,  
		url : tes,
		success : function(result){
			if(result!=-1){
				// resultInfo = result.substring(14,result.length-1);
                var jsonData = JSON.stringify(result);
				resultInfo = jsonData;
			}else{
				resultInfo = result;
			}	
		},  
		error:function(){  
			resultInfo = "{\"Result\":\"-1\",\"ResultMessage\":\"网络问题,请确认服务是否存在\"}";
		}
	})
	return resultInfo;
}
serviceT.getDiskSectorNum = function(pid){
	var resultInfo;
	var tes = "http://127.0.0.1:8090/disk/getDiskSectorNum?diskname="+pid;
    console.log(tes)
	jQuery.support.cors = true;
	$.ajax({  
		type : "get",  
		async:false,  
		url : tes,
		success : function(result){
			if(result!=-1){
				// resultInfo = result.substring(14,result.length-1);
                var jsonData = JSON.stringify(result);
				resultInfo = jsonData;
			}else{
				resultInfo = result;
			}	
		},  
		error:function(){  
			resultInfo = "{\"Result\":\"-1\",\"ResultMessage\":\"网络问题,请确认服务是否存在\"}";
		}
	})
	return resultInfo;
}
serviceT.getPortState = function(pid){
	var resultInfo;
	var tes = "http://127.0.0.1:8090/net/getPortState?port="+pid;
    console.log(tes)
	jQuery.support.cors = true;
	$.ajax({  
		type : "get",  
		async:false,  
		url : tes,
		success : function(result){
			if(result!=-1){
				// resultInfo = result.substring(14,result.length-1);
                var jsonData = JSON.stringify(result);
				resultInfo = jsonData;
			}else{
				resultInfo = result;
			}	
		},  
		error:function(){  
			resultInfo = "{\"Result\":\"-1\",\"ResultMessage\":\"网络问题,请确认服务是否存在\"}";
		}
	})
	return resultInfo;
}
serviceT.getGatewayInfo = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8090/net/getGatewayInfo";
    console.log(tes)
	jQuery.support.cors = true;
	$.ajax({  
		type : "get",  
		async:false,  
		url : tes,
		success : function(result){
			if(result!=-1){
				// resultInfo = result.substring(14,result.length-1);
                var jsonData = JSON.stringify(result);
				resultInfo = jsonData;
			}else{
				resultInfo = result;
			}	
		},  
		error:function(){  
			resultInfo = "{\"Result\":\"-1\",\"ResultMessage\":\"网络问题,请确认服务是否存在\"}";
		}
	})
	return resultInfo;
}
serviceT.getIoSpeed = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8090/runinfo/getIoSpeed";
    console.log(tes)
	jQuery.support.cors = true;
	$.ajax({  
		type : "get",  
		async:false,  
		url : tes,
		success : function(result){
			if(result!=-1){
				// resultInfo = result.substring(14,result.length-1);
                var jsonData = JSON.stringify(result);
				resultInfo = jsonData;
			}else{
				resultInfo = result;
			}	
		},  
		error:function(){  
			resultInfo = "{\"Result\":\"-1\",\"ResultMessage\":\"网络问题,请确认服务是否存在\"}";
		}
	})
	return resultInfo;
}
serviceT.getNetSpeed = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8090/runinfo/getNetSpeed";
    console.log(tes)
	jQuery.support.cors = true;
	$.ajax({  
		type : "get",  
		async:false,  
		url : tes,
		success : function(result){
			if(result!=-1){
				// resultInfo = result.substring(14,result.length-1);
                var jsonData = JSON.stringify(result);
				resultInfo = jsonData;
			}else{
				resultInfo = result;
			}	
		},  
		error:function(){  
			resultInfo = "{\"Result\":\"-1\",\"ResultMessage\":\"网络问题,请确认服务是否存在\"}";
		}
	})
	return resultInfo;
}
serviceT.getMainboardTemperature = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8090/runinfo/getMainboardTemperature";
    console.log(tes)
	jQuery.support.cors = true;
	$.ajax({  
		type : "get",  
		async:false,  
		url : tes,
		success : function(result){
			if(result!=-1){
				// resultInfo = result.substring(14,result.length-1);
                var jsonData = JSON.stringify(result);
				resultInfo = jsonData;
			}else{
				resultInfo = result;
			}	
		},  
		error:function(){  
			resultInfo = "{\"Result\":\"-1\",\"ResultMessage\":\"网络问题,请确认服务是否存在\"}";
		}
	})
	return resultInfo;
}
serviceT.getCpuTemperature = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8090/runinfo/getCpuTemperature";
    console.log(tes)
	jQuery.support.cors = true;
	$.ajax({  
		type : "get",  
		async:false,  
		url : tes,
		success : function(result){
			if(result!=-1){
				// resultInfo = result.substring(14,result.length-1);
                var jsonData = JSON.stringify(result);
				resultInfo = jsonData;
			}else{
				resultInfo = result;
			}	
		},  
		error:function(){  
			resultInfo = "{\"Result\":\"-1\",\"ResultMessage\":\"网络问题,请确认服务是否存在\"}";
		}
	})
	return resultInfo;
}
serviceT.getDiskTemperature = function(pid){
	var resultInfo;
	var tes = "http://127.0.0.1:8090/runinfo/getDiskTemperature?diskpath="+pid;
    console.log(tes)
	jQuery.support.cors = true;
	$.ajax({  
		type : "get",  
		async:false,  
		url : tes,
		success : function(result){
			if(result!=-1){
				// resultInfo = result.substring(14,result.length-1);
                var jsonData = JSON.stringify(result);
				resultInfo = jsonData;
			}else{
				resultInfo = result;
			}	
		},  
		error:function(){  
			resultInfo = "{\"Result\":\"-1\",\"ResultMessage\":\"网络问题,请确认服务是否存在\"}";
		}
	})
	return resultInfo;
}
serviceT.getGPSInfo = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8090/gps/getGPSInfo";
    console.log(tes)
	jQuery.support.cors = true;
	$.ajax({  
		type : "get",  
		async:false,  
		url : tes,
		success : function(result){
			if(result!=-1){
				// resultInfo = result.substring(14,result.length-1);
                var jsonData = JSON.stringify(result);
				resultInfo = jsonData;
			}else{
				resultInfo = result;
			}	
		},  
		error:function(){  
			resultInfo = "{\"Result\":\"-1\",\"ResultMessage\":\"网络问题,请确认服务是否存在\"}";
		}
	})
	return resultInfo;
}
serviceT.getProcInfoCpuUsage = function(pid){
	var resultInfo;
	var tes = "http://127.0.0.1:8090/process/getProcInfoCpuUsage?pid="+pid;
    console.log(tes)
	jQuery.support.cors = true;
	$.ajax({  
		type : "get",  
		async:false,  
		url : tes,
		success : function(result){
			if(result!=-1){
				// resultInfo = result.substring(14,result.length-1);
                var jsonData = JSON.stringify(result);
				resultInfo = jsonData;
			}else{
				resultInfo = result;
			}	
		},  
		error:function(){  
			resultInfo = "{\"Result\":\"-1\",\"ResultMessage\":\"网络问题,请确认服务是否存在\"}";
		}
	})
	return resultInfo;
}
serviceT.getProcInfoIoUsage = function(pid){
	var resultInfo;
	var tes = "http://127.0.0.1:8090/process/getProcInfoIoUsage?pid="+pid;
    console.log(tes)
	jQuery.support.cors = true;
	$.ajax({  
		type : "get",  
		async:false,  
		url : tes,
		success : function(result){
			if(result!=-1){
				// resultInfo = result.substring(14,result.length-1);
                var jsonData = JSON.stringify(result);
				resultInfo = jsonData;
			}else{
				resultInfo = result;
			}	
		},  
		error:function(){  
			resultInfo = "{\"Result\":\"-1\",\"ResultMessage\":\"网络问题,请确认服务是否存在\"}";
		}
	})
	return resultInfo;
}
serviceT.getProcInfoMemUsage = function(pid){
	var resultInfo;
	var tes = "http://127.0.0.1:8090/process/getProcInfoMemUsage?pid="+pid;
    console.log(tes)
	jQuery.support.cors = true;
	$.ajax({  
		type : "get",  
		async:false,  
		url : tes,
		success : function(result){
			if(result!=-1){
				// resultInfo = result.substring(14,result.length-1);
                var jsonData = JSON.stringify(result);
				resultInfo = jsonData;
			}else{
				resultInfo = result;
			}	
		},  
		error:function(){  
			resultInfo = "{\"Result\":\"-1\",\"ResultMessage\":\"网络问题,请确认服务是否存在\"}";
		}
	})
	return resultInfo;
}
serviceT.getProcInfoStatus = function(pid){
	var resultInfo;
	var tes = "http://127.0.0.1:8090/process/getProcInfoStatus?pid="+pid;
    console.log(tes)
	jQuery.support.cors = true;
	$.ajax({  
		type : "get",  
		async:false,  
		url : tes,
		success : function(result){
			if(result!=-1){
				// resultInfo = result.substring(14,result.length-1);
                var jsonData = JSON.stringify(result);
				resultInfo = jsonData;
			}else{
				resultInfo = result;
			}	
		},  
		error:function(){  
			resultInfo = "{\"Result\":\"-1\",\"ResultMessage\":\"网络问题,请确认服务是否存在\"}";
		}
	})
	return resultInfo;
}
serviceT.getProcInfoPort = function(pid){
	var resultInfo;
	var tes = "http://127.0.0.1:8090/process/getProcInfoPort?pid="+pid;
    console.log(tes)
	jQuery.support.cors = true;
	$.ajax({  
		type : "get",  
		async:false,  
		url : tes,
		success : function(result){
			if(result!=-1){
				// resultInfo = result.substring(14,result.length-1);
                var jsonData = JSON.stringify(result);
				resultInfo = jsonData;
			}else{
				resultInfo = result;
			}	
		},  
		error:function(){  
			resultInfo = "{\"Result\":\"-1\",\"ResultMessage\":\"网络问题,请确认服务是否存在\"}";
		}
	})
	return resultInfo;
}
serviceT.getProcInfoStartTime = function(pid){
	var resultInfo;
	var tes = "http://127.0.0.1:8090/process/getProcInfoStartTime?pid="+pid;
    console.log(tes)
	jQuery.support.cors = true;
	$.ajax({  
		type : "get",  
		async:false,  
		url : tes,
		success : function(result){
			if(result!=-1){
				// resultInfo = result.substring(14,result.length-1);
                var jsonData = JSON.stringify(result);
				resultInfo = jsonData;
			}else{
				resultInfo = result;
			}	
		},  
		error:function(){  
			resultInfo = "{\"Result\":\"-1\",\"ResultMessage\":\"网络问题,请确认服务是否存在\"}";
		}
	})
	return resultInfo;
}
serviceT.getProcInfoRunningTime = function(pid){
	var resultInfo;
	var tes = "http://127.0.0.1:8090/process/getProcInfoRunningTime?pid="+pid;
    console.log(tes)
	jQuery.support.cors = true;
	$.ajax({  
		type : "get",  
		async:false,  
		url : tes,
		success : function(result){
			if(result!=-1){
				// resultInfo = result.substring(14,result.length-1);
                var jsonData = JSON.stringify(result);
				resultInfo = jsonData;
			}else{
				resultInfo = result;
			}	
		},  
		error:function(){  
			resultInfo = "{\"Result\":\"-1\",\"ResultMessage\":\"网络问题,请确认服务是否存在\"}";
		}
	})
	return resultInfo;
}
serviceT.getProcInfoCpuTime = function(pid){
	var resultInfo;
	var tes = "http://127.0.0.1:8090/process/getProcInfoCpuTime?pid="+pid;
    console.log(tes)
	jQuery.support.cors = true;
	$.ajax({  
		type : "get",  
		async:false,  
		url : tes,
		success : function(result){
			if(result!=-1){
				// resultInfo = result.substring(14,result.length-1);
                var jsonData = JSON.stringify(result);
				resultInfo = jsonData;
			}else{
				resultInfo = result;
			}	
		},  
		error:function(){  
			resultInfo = "{\"Result\":\"-1\",\"ResultMessage\":\"网络问题,请确认服务是否存在\"}";
		}
	})
	return resultInfo;
}
serviceT.getProcInfoCmd = function(pid){
	var resultInfo;
	var tes = "http://127.0.0.1:8090/process/getProcInfoCmd?pid="+pid;
    console.log(tes)
	jQuery.support.cors = true;
	$.ajax({  
		type : "get",  
		async:false,  
		url : tes,
		success : function(result){
			if(result!=-1){
				// resultInfo = result.substring(14,result.length-1);
                var jsonData = JSON.stringify(result);
				resultInfo = jsonData;
			}else{
				resultInfo = result;
			}	
		},  
		error:function(){  
			resultInfo = "{\"Result\":\"-1\",\"ResultMessage\":\"网络问题,请确认服务是否存在\"}";
		}
	})
	return resultInfo;
}
serviceT.getProcInfoUser = function(pid){
	var resultInfo;
	var tes = "http://127.0.0.1:8090/process/getProcInfoUser?pid="+pid;
    console.log(tes)
	jQuery.support.cors = true;
	$.ajax({  
		type : "get",  
		async:false,  
		url : tes,
		success : function(result){
			if(result!=-1){
				// resultInfo = result.substring(14,result.length-1);
                var jsonData = JSON.stringify(result);
				resultInfo = jsonData;
			}else{
				resultInfo = result;
			}	
		},  
		error:function(){  
			resultInfo = "{\"Result\":\"-1\",\"ResultMessage\":\"网络问题,请确认服务是否存在\"}";
		}
	})
	return resultInfo;
}
serviceT.getProcInfo = function(vid){
	var resultInfo;
	var tes = "http://127.0.0.1:8090/process/getProcInfo?processname="+vid;
    console.log(tes)
	jQuery.support.cors = true;
	$.ajax({  
		type : "get",  
		async:false,  
		url : tes,
		success : function(result){
			if(result!=-1){
				// resultInfo = result.substring(14,result.length-1);
                var jsonData = JSON.stringify(result);
				resultInfo = jsonData;
			}else{
				resultInfo = result;
			}	
		},  
		error:function(){  
			resultInfo = "{\"Result\":\"-1\",\"ResultMessage\":\"网络问题,请确认服务是否存在\"}";
		}
	})
	return resultInfo;
}
