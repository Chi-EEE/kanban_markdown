import { defineConfig } from 'vite';
import solidPlugin from 'vite-plugin-solid';

export default defineConfig({
    plugins: [
        solidPlugin(),
    ],
    server: {
        port: 3000,
    },
    build: {
        target: 'esnext',
        rollupOptions: {
            output: {
                dir: 'out/',
                entryFileNames: 'compiled/bundled.js',
                assetFileNames: 'compiled/bundled.css',
                chunkFileNames: "chunk.js",
                manualChunks: undefined,
            }
        }
    },
});
