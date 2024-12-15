effects.windowAoceand.connect(function (window) {
    if (effect.grab(window, Effect.WindowAoceandGrabRole, true)) {
        sendTestResponse('ok');
    } else {
        sendTestResponse('fail');
    }
});
