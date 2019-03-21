{
    "targets": [
        {
            "target_name": "VideoRecorder",
            "sources": [ "VideoRecorder.mm" ],
            "link_settings": {
                "libraries": [
                    "$(SDKROOT)/System/Library/Frameworks/Cocoa.framework",
                    "$(SDKROOT)/System/Library/Frameworks/AVFoundation.framework"
                ]
            },
        }
    ]
}