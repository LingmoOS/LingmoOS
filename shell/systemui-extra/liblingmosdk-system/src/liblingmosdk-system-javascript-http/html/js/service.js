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
	var tes = "http://127.0.0.1:8888/resolution/getSysLegalResolution";
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
	var tes = "http://127.0.0.1:8888/netLink/getNetState";
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
	var tes = "http://127.0.0.1:8888/peripheralsenum/getAllUsbInfo";
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
	var tes = "http://127.0.0.1:8888/netcard/getNetCardName";
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
serviceT.getNetCardUpcards = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/netcard/getNetCardUpcards";
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
serviceT.getNetCardUp = function(pid){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/netcard/getNetCardUp?netCardName="+pid;
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
serviceT.getNetCardPhymac = function(pid){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/netcard/getNetCardPhymac?netCardName="+pid;
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
serviceT.getNetCardPrivateIPv4 = function(pid){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/netcard/getNetCardPrivateIPv4?netCardName="+pid;
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
serviceT.getNetCardIPv4 = function(pid){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/netcard/getNetCardIPv4?netCardName="+pid;
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
serviceT.getNetCardPrivateIPv6 = function(pid){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/netcard/getNetCardPrivateIPv6?netCardName="+pid;
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
serviceT.getNetCardType = function(pid){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/netcard/getNetCardType?netCardName="+pid;
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
	var tes = "http://127.0.0.1:8888/netcard/getNetCardProduct?netCardName="+pid;
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
serviceT.getNetCardIPv6 = function(pid){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/netcard/getNetCardIPv6?netCardName="+pid;
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
serviceT.getBiosVendor = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/bios/getBiosVendor";
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
serviceT.getBiosVersion = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/bios/getBiosVersion";
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
	var tes = "http://127.0.0.1:8888/mainboard/getMainboardName";
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
	var tes = "http://127.0.0.1:8888/mainboard/getMainboardDate";
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
	var tes = "http://127.0.0.1:8888/mainboard/getMainboardSerial";
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
	var tes = "http://127.0.0.1:8888/mainboard/getMainboardVendor";
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
serviceT.getDiskList = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/disk/getDiskList";
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
serviceT.getDiskSectorSize = function(pid){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/disk/getDiskSectorSize?diskname="+pid;
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
serviceT.getDiskTotalSizeMiB = function(pid){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/disk/getDiskTotalSizeMiB?diskname="+pid;
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
serviceT.getDiskModel = function(pid){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/disk/getDiskModel?diskname="+pid;
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
serviceT.getDiskSerial = function(pid){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/disk/getDiskSerial?diskname="+pid;
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
serviceT.getDiskType = function(pid){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/disk/getDiskType?diskname="+pid;
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
	var tes = "http://127.0.0.1:8888/disk/getDiskVersion?diskname="+pid;
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
serviceT.getDiskPartitionNums = function(pid){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/disk/getDiskPartitionNums?diskname="+pid;
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
	var tes = "http://127.0.0.1:8888/disk/getDiskSectorNum?diskname="+pid;
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
	var tes = "http://127.0.0.1:8888/net/getPortState?port="+pid;
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
serviceT.getMultiplePortStat = function(pid,vid){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/net/getMultiplePortStat?startid="+pid+"&endid="+vid;
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
	var tes = "http://127.0.0.1:8888/net/getGatewayInfo";
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
serviceT.getFirewallState = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/net/getFirewallState";
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
	var tes = "http://127.0.0.1:8888/runinfo/getIoSpeed";
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
serviceT.getNetSpeed = function(pid){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/runinfo/getNetSpeed?netname="+pid;
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
	var tes = "http://127.0.0.1:8888/runinfo/getMainboardTemperature";
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
	var tes = "http://127.0.0.1:8888/runinfo/getCpuTemperature";
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
serviceT.getDiskRate = function(pid){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/runinfo/getDiskRate?diskpath="+pid;
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
	var tes = "http://127.0.0.1:8888/runinfo/getDiskTemperature?diskpath="+pid;
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
	var tes = "http://127.0.0.1:8888/gps/getGPSInfo";
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
serviceT.getAllProcInfo = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/process/getAllProcInfo";
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
	var tes = "http://127.0.0.1:8888/process/getProcInfoCpuUsage?pid="+pid;
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
	var tes = "http://127.0.0.1:8888/process/getProcInfoIoUsage?pid="+pid;
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
	var tes = "http://127.0.0.1:8888/process/getProcInfoMemUsage?pid="+pid;
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
	var tes = "http://127.0.0.1:8888/process/getProcInfoStatus?pid="+pid;
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
	var tes = "http://127.0.0.1:8888/process/getProcInfoPort?pid="+pid;
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
	var tes = "http://127.0.0.1:8888/process/getProcInfoStartTime?pid="+pid;
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
	var tes = "http://127.0.0.1:8888/process/getProcInfoRunningTime?pid="+pid;
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
	var tes = "http://127.0.0.1:8888/process/getProcInfoCpuTime?pid="+pid;
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
	var tes = "http://127.0.0.1:8888/process/getProcInfoCmd?pid="+pid;
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
	var tes = "http://127.0.0.1:8888/process/getProcInfoUser?pid="+pid;
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
	var tes = "http://127.0.0.1:8888/process/getProcInfo?processname="+vid;
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
serviceT.getCpuArch = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/cpuinfo/getCpuArch";
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
serviceT.getCpuVendor = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/cpuinfo/getCpuVendor";
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
serviceT.getCpuModel = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/cpuinfo/getCpuModel";
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
serviceT.getCpuFreqMHz = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/cpuinfo/getCpuFreqMHz";
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
serviceT.getCpuCorenums = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/cpuinfo/getCpuCorenums";
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
serviceT.getCpuVirt = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/cpuinfo/getCpuVirt";
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
serviceT.getCpuProcess = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/cpuinfo/getCpuProcess";
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
serviceT.getPackageList = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/packageinfo/getPackageList";
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
serviceT.getPackageVersion = function(pid){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/packageinfo/getPackageVersion?packagename="+pid;
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
serviceT.getPackageInstalled = function(pid,vid){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/packageinfo/getPackageInstalled?packagename="+pid+"&packageversion="+vid;
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
serviceT.getMemTotalKiB = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/resource/getMemTotalKiB";
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
serviceT.getMemUsagePercent = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/resource/getMemUsagePercent";
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
serviceT.getMemUsageKiB = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/resource/getMemUsageKiB";
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
serviceT.getMemAvailableKiB = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/resource/getMemAvailableKiB";
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
serviceT.getMemFreeKiB = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/resource/getMemFreeKiB";
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
serviceT.getMemVirtAllocKiB = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/resource/getMemVirtAllocKiB";
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
serviceT.getMemSwapTotalKiB = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/resource/getMemSwapTotalKiB";
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
serviceT.getMemSwapUsagePercent = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/resource/getMemSwapUsagePercent";
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
serviceT.getMemSwapUsageKiB = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/resource/getMemSwapUsageKiB";
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
serviceT.getMemSwapFreeKiB = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/resource/getMemSwapFreeKiB";
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
serviceT.getCpuCurrentUsage = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/resource/getCpuCurrentUsage";
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
serviceT.getUpTime = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/resource/getUpTime";
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
serviceT.getSystemArchitecture = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/sysinfo/getSystemArchitecture";
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
serviceT.getSystemName = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/sysinfo/getSystemName";
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
serviceT.getSystemActivationStatus = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/sysinfo/getSystemActivationStatus";
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
serviceT.getSystemSerialNumber = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/sysinfo/getSystemSerialNumber";
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
serviceT.getSystemKernelVersion = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/sysinfo/getSystemKernelVersion";
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
serviceT.getSystemEffectUser = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/sysinfo/getSystemEffectUser";
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
serviceT.getSystemProjectName = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/sysinfo/getSystemProjectName";
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
serviceT.getSystemProjectSubName = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/sysinfo/getSystemProjectSubName";
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
serviceT.getSystemProductFeatures = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/sysinfo/getSystemProductFeatures";
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
serviceT.getSystemHostVirtType = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/sysinfo/getSystemHostVirtType";
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
serviceT.getSystemHostCloudPlatform = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/sysinfo/getSystemHostCloudPlatform";
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
serviceT.getSystemOSVersion = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/sysinfo/getSystemOSVersion";
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
serviceT.getSystemUpdateVersion = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/sysinfo/getSystemUpdateVersion";
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
serviceT.getSystemIsZYJ = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/sysinfo/getSystemIsZYJ";
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
serviceT.getSysLegalResolution = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/sysinfo/getSysLegalResolution";
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
serviceT.getSystemVersion = function(pid){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/sysinfo/getSystemVersion?systemversion="+pid;
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
serviceT.getPrinterList = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/printer/getPrinterList";
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
serviceT.getPrinterAvailableList = function(){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/printer/getPrinterAvailableList";
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
serviceT.setPrinterOptions = function(pid,vid,xid,yid){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/printer/setPrinterOptions?pagenum="+pid+"&pagetype="+vid+"&cputype="+xid+"&sidetype="+yid;
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
serviceT.getPrinterPrintLocalFile = function(aid,bid){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/printer/getPrinterPrintLocalFile?printername="+aid+"&filepath="+bid;
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
serviceT.getPrinterCancelAllJobs = function(cid){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/printer/getPrinterCancelAllJobs?printername="+cid;
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
serviceT.getPrinterStatus = function(cid){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/printer/getPrinterStatus?printername="+cid;
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
serviceT.getPrinterFilename = function(cid){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/printer/getPrinterFilename?printername="+cid;
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
serviceT.getPrinterJobStatus = function(cid,vid){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/printer/getPrinterJobStatus?printername="+cid+"&jobid="+vid;
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
serviceT.getPrinterDownloadRemoteFile = function(cid,vid){
	var resultInfo;
	var tes = "http://127.0.0.1:8888/printer/getPrinterDownloadRemoteFile?file="+cid+"&filepath="+vid;
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
