/*
 *  SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

export class ImageData {
    constructor({ author_name, author_url, image_description, image_page, image_url }) {
        this.author_name = author_name;
        this.author_url = author_url;
        this.image_description = image_description;
        this.image_page = image_page;
        this.image_url = image_url;
    }
}

export class FileProvider {
    constructor(fileUrl) {
        this.fileUrl = fileUrl;
    }

    imageData() {
        return new ImageData({
            author_name: "Unknown",
            author_url: "",
            image_description: decodeURI(this.fileUrl).replace(/.*\//, ""),
            image_page: this.fileUrl,
            image_url: this.fileUrl,
        });
    }
}

// A little bit obfuscated to prevent simple grep
const defaultUnsplashAccessKey = Qt.atob("LWlTR3FPbXJYeTY1LW9ncU1uNGtNe" + "TRlaDZXUmZSVVdDMGdrNllublh2Zw==");

export class UnsplashProvider {
    constructor(accessKey = defaultUnsplashAccessKey, apiUrl = "https://api.unsplash.com") {
        this.apiUrl = apiUrl;
        this.accessKey = accessKey;
    }

    // callback: (ImageData) -> void
    getRandom(callback) {
        const xhr = this.__openRequest("GET", "/photos/random");
        xhr.onreadystatechange = () => {
            if (xhr.readyState === XMLHttpRequest.DONE && xhr.status === 200) {
                const json = JSON.parse(xhr.response);
                const imageData = new ImageData({
                    author_name: json.user.name,
                    author_url: json.user.links.self,
                    image_description: json.alt_description,
                    image_page: json.links.html,
                    image_url: json.urls.regular,
                });
                callback(imageData);
            }
        };
        xhr.send();
        return xhr;
    }

    __openRequest(method, endpoint) {
        const xhr = new XMLHttpRequest();
        xhr.open(method, `${this.apiUrl}${endpoint}`);
        xhr.setRequestHeader("Accept-Version", "v1");
        xhr.setRequestHeader("Authorization", `Client-ID ${this.accessKey}`);
        return xhr;
    }
}
