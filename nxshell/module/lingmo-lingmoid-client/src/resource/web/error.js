"use strict";

var myStatus = response.status;
console.log(myStatus);

if (myStatus !== 200) {
    window.location.href = "http://www.w3schools.com";
}
