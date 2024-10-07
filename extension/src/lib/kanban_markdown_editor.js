const { getNonce } = require('./util');
const zlib = require('zlib');
const fs = require('fs');

const vscode = require('vscode');
const { WasmContext } = require('@vscode/wasm-component-model')
// const { KanbanMarkdownServer } = require('./kanban_markdown_server');

/**
 * @implements {vscode.CustomTextEditorProvider}
 */
class KanbanMarkdownEditorProvider {

    /**
     * @public
     * @param {vscode.ExtensionContext} context 
     * @returns {vscode.Disposable}
     */
    static async register(context) {
        const provider = new KanbanMarkdownEditorProvider(context);
        const providerRegistration = vscode.window.registerCustomEditorProvider(KanbanMarkdownEditorProvider.viewType, provider);
        
        const filename = vscode.Uri.joinPath(context.extensionUri, 'target', 'wasm32-unknown-unknown', 'debug', 'calculator.wasm');
        const bits = await vscode.workspace.fs.readFile(filename);
        const module = await WebAssembly.compile(bits);

        // The context for the WASM module
        const wasmContext = new WasmContext.Default();

        // Create the bindings to import the log function into the WASM module
        const imports = calculator._.imports.create(service, wasmContext);
        // Instantiate the module
        const instance = await WebAssembly.instantiate(module, imports);

        // Bind the WASM memory to the context
        wasmContext.initialize(new Memory.Default(instance.exports));
        
        return providerRegistration;
    }

    /**
     * @private
     * @readonly
     */
    static viewType = 'kanban-markdown.editor';

    /**
     * @param {vscode.ExtensionContext} context 
     */
    constructor(context) {
        this.context = context;
    }

    /**
     * @public
     * @param {vscode.TextDocument} document 
     * @param {vscode.WebviewPanel} webviewPanel 
     * @param {vscode.CancellationToken} token 
     */
    resolveCustomTextEditor(document, webviewPanel, token) {
        // Setup initial content for the webview
        // This cannot be done in the constructor because the webviewPanel is not available yet
        fs.readFile(document.uri.fsPath, 'binary', (err, data) => {
            /**
             * @param {any} data
             */
            function updateWebview(data) {
                webviewPanel.webview.postMessage({
                    type: 'update',
                    text: JSON.stringify(data),
                });
            }

            webviewPanel.webview.options = {
                enableScripts: true,
            };

            webviewPanel.webview.html = this.getHtmlForWebview(webviewPanel.webview);

            console.log('1:');
            console.log('Module:', Module);
            console.log('2:');
            const maybe_kanban_board = Module.parse(data);
            console.log('Received data:', maybe_kanban_board.ok);
            if (!maybe_kanban_board.ok) {
                console.error('Failed to parse file:', maybe_kanban_board.err);
                return;
            }
            let kanban_board = maybe_kanban_board.value;

            const changeDocumentSubscription = vscode.workspace.onDidChangeTextDocument(e => {
                if (e.document.uri.toString() === document.uri.toString()) {
                    kanban_board = Module.parse(e.document.getText()).value;
                    updateWebview(Module.json_format_str(kanban_board));
                }
            });

            // Make sure we get rid of the listener when our editor is closed.
            webviewPanel.onDidDispose(() => {
                server.close();
                changeDocumentSubscription.dispose();
            });

            webviewPanel.webview.onDidReceiveMessage(e => {
                Module.update(kanban_board, JSON.stringify({
                    type: 'commands',
                    commands: e.commands,
                }))
                this.updateTextDocument(document, Module.markdown_format_str(kanban_board));
            });

            updateWebview(Module.json_format_str(kanban_board));
        });
    }

    /**
     * @private
     * @param {vscode.Webview} webview 
     * @returns {string}
     */
    getHtmlForWebview(webview) {
        // Local path to script and css for the webview
        const scriptUri = webview.asWebviewUri(vscode.Uri.joinPath(
            this.context.extensionUri, 'dist', 'frontend', 'bundled.js'));

        const styleMainUri = webview.asWebviewUri(vscode.Uri.joinPath(
            this.context.extensionUri, 'dist', 'frontend', 'bundled.css'));
        
        // Use a nonce to whitelist which scripts can be run
        const nonce = getNonce();

        // To allow SolidJS: https://github.com/withastro/astro/pull/2359
        return /* html */`
        <!DOCTYPE html>
        <html lang="en">
            <head>
                <meta charset="UTF-8">
                <meta http-equiv="Content-Security-Policy" content="default-src 'none'; img-src ${webview.cspSource}; style-src ${webview.cspSource} 'unsafe-inline'; script-src 'nonce-${nonce}';">
                <meta name="viewport" content="width=device-width, initial-scale=1.0">
                <script nonce="${nonce}">window._$HY||(_$HY={events:[],completed:new WeakSet,r:{}})</script>
                <script type="module" nonce="${nonce}" crossorigin src="${scriptUri}"></script>
                <link href="${styleMainUri}" crossorigin rel="stylesheet" />
                <title>Kanban Markdown</title>
            </head>
            <body>
                <script nonce="${nonce}">
                    const vscode = acquireVsCodeApi();
                </script>
                <noscript>You need to enable JavaScript to run this app.</noscript>
                <div id="root"></div>
            </body>
        </html>
`;
    }

    /**
     * @private
     * @param {vscode.TextDocument} document 
     * @param {string} markdown
     * @returns 
     */
    updateTextDocument(document, markdown) {
        const edit = new vscode.WorkspaceEdit();

        // Just replace the entire document every time for this example extension.
        // A more complete extension should compute minimal edits instead.
        edit.replace(
            document.uri,
            new vscode.Range(0, 0, document.lineCount, 0),
            markdown);

        return vscode.workspace.applyEdit(edit);
    }
}

module.exports = {
    KanbanMarkdownEditorProvider
}