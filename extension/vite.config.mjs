import { defineConfig } from 'vite';
import solidPlugin from 'vite-plugin-solid';
import { obfuscator as rollupObfuscator } from 'rollup-obfuscator';

/**
 * Obfuscator Config 
 * @see {@link https://github.com/javascript-obfuscator/javascript-obfuscator#preset-options}
 */
const obfuscatorConfig = {
    compact: true,
    controlFlowFlattening: true,
    controlFlowFlatteningThreshold: 0.75,
    deadCodeInjection: true,
    deadCodeInjectionThreshold: 0.4,
    debugProtection: false,
    debugProtectionInterval: 0,
    disableConsoleOutput: true,
    identifierNamesGenerator: 'hexadecimal',
    log: false,
    numbersToExpressions: true,
    renameGlobals: false,
    selfDefending: true,
    simplify: true,
    splitStrings: true,
    splitStringsChunkLength: 10,
    stringArray: true,
    stringArrayCallsTransform: true,
    stringArrayCallsTransformThreshold: 0.75,
    stringArrayEncoding: ['base64'],
    stringArrayIndexShift: true,
    stringArrayRotate: true,
    stringArrayShuffle: true,
    stringArrayWrappersCount: 2,
    stringArrayWrappersChainedCalls: true,
    stringArrayWrappersParametersMaxCount: 4,
    stringArrayWrappersType: 'function',
    stringArrayThreshold: 0.75,
    transformObjectKeys: true,
    unicodeEscapeSequence: false
}

export default defineConfig({
    plugins: [
        solidPlugin(),
        rollupObfuscator({
            global: true,
            exclude: ['./node_modules/**/*'],
            ...obfuscatorConfig,
        })
    ],
    server: {
        port: 3000,
    },
    build: {
        target: 'esnext',
        rollupOptions: {
            output: {
                dir: 'dist/frontend/',
                entryFileNames: 'bundled.js',
                assetFileNames: 'bundled.css',
                chunkFileNames: "chunk.js",
                manualChunks: undefined,
            }
        }
    },
});
