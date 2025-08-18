/** @type {import('vite').UserConfig} */

import { defineConfig } from 'vite'
import { viteStaticCopy } from 'vite-plugin-static-copy'

// relative to 'root'
const iconsPath = '../node_modules/@shoelace-style/shoelace/dist/assets/icons'

// https://vitejs.dev/config/
export default defineConfig({
    root: 'www',
    resolve: {
        alias: [
            {
                find: /\/assets\/icons\/(.+)/,
                replacement: `${iconsPath}/$1`,
            },
        ],
    },
    plugins: [
        viteStaticCopy({
            targets: [
                {
                    src: iconsPath,
                    dest: 'assets',
                },
            ],
        }),
    ],
});
