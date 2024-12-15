effects.windowAoceand.connect(function (window) {
    if (effect.grab(window, Effect.WindowAoceandGrabRole)) {
        sendTestResponse('ok');
    } else {
        sendTestResponse('fail');
    }
});
