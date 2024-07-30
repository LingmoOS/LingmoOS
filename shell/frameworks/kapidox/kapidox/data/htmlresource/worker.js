/**
 * SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

const ServiceWorkerVersion = 0
const AssetCacheName = `assets-${ServiceWorkerVersion}`

self.addEventListener("install", event => {
    event.waitUntil(
        caches.open(AssetCacheName).then(cache => {
            return cache.addAll([
                "/resources",
                "/index.html",
            ])
        })
    )
})

self.addEventListener("activate", event => {
    event.waitUntil(
        caches.keys().then(cacheNames => {
            return Promise.all(
                cacheNames.map(cacheName => {
                    if (cacheName != AssetCacheName) {
                        return caches.delete(cacheName)
                    }
                })
            )
        })
    )
})

self.addEventListener("fetch", event => {
    event.respondWith(
        caches.open(AssetCacheName).then(cache => {
            return cache.match(event.request).then(response => {
                if (response) {
                    return response
                }

                return fetch(event.request.clone()).then(requestResponse => {
                    if (requestResponse.status < 400) {
                        cache.put(event.request, requestResponse.clone())
                    }

                    return requestResponse
                })
            }, error => {
                console.log(`Error in cache match: ${error}`)
                throw error
            })
        }, error => {
            console.log(`Error in cache opening: ${error}`)
            throw error
        })
    )
})

self.addEventListener("message", (ev) => {
    if (ev.data && ev.data.type === "CONSIDER_CACHING") {
        let url = ev.data.url
        if (url.includes('/html/')) {
            let toCache = url.split('/html/')[0]
            caches.open(AssetCacheName).then(cache => {
                return cache.addAll([
                    toCache
                ])
            })
        }
    }
})
