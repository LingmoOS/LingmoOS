"use strict";

function updatePlatforms() {
    var doFilter = document.getElementById("platform-filter").checked;
    $(".library-platform").removeClass("library-platform-required");
    if (doFilter) {
        $("#platform-filter-group").show();
    } else {
        $("#platform-filter-group").hide();
        $(".library-row").removeClass("not-available").removeClass("available");
        return;
    }

    var platformCheckboxes = $(".platform-checkbox");
    var wantedPlatforms = [];
    platformCheckboxes.each(function(idx, checkbox) {
        if (checkbox.checked) {
            var platform = checkbox.getAttribute("data-platform");
            wantedPlatforms.push(platform);
        }
    });
    for (var idx=0; idx < wantedPlatforms.length; ++idx) {
        var platform = wantedPlatforms[idx];
        $(".library-platform-" + platform).addClass("library-platform-required");
    };
    $(".library-row").each(function(idx, tr) {
        var fwPlatforms = tr.getAttribute("data-platforms").split(",");
        var show = wantedPlatforms.every(function(platform) {
            return fwPlatforms.indexOf(platform) != -1;
        });
        if (show) {
            $(tr).removeClass("not-available").addClass("available");
        } else {
            $(tr).removeClass("available").addClass("not-available");
        }
    });
}

var g_noteTip;
var g_currentlyDescribedElement;

function initNoteTip() {
    g_noteTip = new NoteTip();
    g_noteTip.onHide = function() {
        g_currentlyDescribedElement = null;
    }

    $(".library-platform > a").click(function() {
        if (g_currentlyDescribedElement == this) {
            g_noteTip.hide();
        } else {
            g_currentlyDescribedElement = this;
            var text = this.getAttribute("data-note");
            g_noteTip.setText(text);
            g_noteTip.show(this);
        }
        return false;
    });
}

function main() {
    $("#platform-filter").click(updatePlatforms);
    $(".platform-checkbox").click(updatePlatforms);
    initNoteTip();
}
