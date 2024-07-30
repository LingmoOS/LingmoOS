"use strict";

function NoteTip() {
    this.element = document.createElement("div");
    $(this.element).addClass("note-tip");
    $(this.element).html("<span class='note-tip-text'></span> <a class='note-close' href='#'>&#9447;</a>");
    $("body").append(this.element);

    var that = this;
    $(".note-close", this.element).click(function() {
        that.hide();
        return false;
    });
}

NoteTip.prototype.element = null;

NoteTip.prototype.onHide = null;

NoteTip.prototype.hide = function() {
    $(this.element).fadeOut();
    if (this.onHide) {
        this.onHide();
    }
}

NoteTip.prototype.setText = function(text) {
    $(".note-tip-text", this.element).text(text);
}

NoteTip.prototype.show = function(anchorElement) {
    var offset = $(anchorElement).offset();
    var elt = $(this.element);
    elt.css({
            left: (offset.left + $(anchorElement).width() / 2 - elt.outerWidth(true) / 2) + "px",
            top: (offset.top - elt.outerHeight(true) - 6) + "px"
        })
        .fadeIn();
}
