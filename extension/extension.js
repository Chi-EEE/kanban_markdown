// The module 'vscode' contains the VS Code extensibility API

const { KanbanMarkdownEditorProvider } = require('./src/KanbanMarkdownEditor');

// Import the module and reference it with the alias vscode in your code below
const vscode = require('vscode');

// This method is called when your extension is activated
// Your extension is activated the very first time the command is executed

const fs = require('fs');
var spawn = require('child_process').spawn;



/**
 * @param {vscode.ExtensionContext} context
 */
function activate(context) {
	context.subscriptions.push(KanbanMarkdownEditorProvider.register(context));

	// Use the console to output diagnostic information (console.log) and errors (console.error)
	// This line of code will only be executed once when your extension is activated
	console.log('Congratulations, your extension "kanban-markdown" is now active!');

	// Check if server exists
	const server_path = vscode.Uri.joinPath(context.extensionUri, 'server', 'kanban-markdown_server.exe');
	if (!fs.existsSync(server_path.fsPath)) {
		console.log("Server not found");
		return;
	}

	var kanban_markdown_server = spawn(server_path.fsPath);
	kanban_markdown_server.stdout.on('data', function (data) {
		console.log('stdout: ' + data.toString());
	});

	kanban_markdown_server.stdin.cork();
	kanban_markdown_server.stdin.write("console.log('Hello from PhantomJS')\n");
	kanban_markdown_server.stdin.uncork();

	// The command has been defined in the package.json file
	// Now provide the implementation of the command with  registerCommand
	// The commandId parameter must match the command field in package.json
	const disposable = vscode.commands.registerCommand('kanban-markdown.helloWorld', function () {
		// The code you place here will be executed every time your command is executed

		// Display a message box to the user
		vscode.window.showInformationMessage('Hello World from Kanban Markdown!');
	});

	context.subscriptions.push(disposable);
}

// This method is called when your extension is deactivated
function deactivate() { }

module.exports = {
	activate,
	deactivate
}
