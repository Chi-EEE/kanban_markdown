/**
 * @typedef {Object} KanbanBoard
 * @property {string} name - The name of the kanban board.
 * @property {Properties} properties - The properties of the kanban board.
 * @property {string} description - The description of the kanban board.
 * @property {Label[]} labels - The labels associated with the kanban board.
 * @property {List[]} lists - The lists on the kanban board.
 */

/**
 * @typedef {Object} Properties
 * @property {string} color - The color of the kanban board.
 * @property {number} created - The timestamp when the kanban board was created.
 * @property {number} last_modified - The timestamp when the kanban board was last modified.
 * @property {number} version - The version of the kanban board.
 * @property {string} checksum - The checksum for the kanban board.
 */

/**
 * @typedef {Object} Label
 * @property {string} name - The name of the label.
 * @property {Task[]} tasks - The tasks associated with the label.
 */

/**
 * @typedef {Object} List
 * @property {string} name - The name of the list.
 * @property {Task[]} tasks - The tasks within the list.
 */

/**
 * @typedef {Object} Task
 * @property {string} name - The name of the task.
 * @property {boolean} [checked] - The status of the task (checked or not). Optional.
 * @property {string[]} [description] - The description of the task. Optional.
 * @property {Label[]} [labels] - The labels associated with the task. Optional.
 * @property {any[]} [attachments] - The attachments of the task. Optional.
 * @property {any[]} [checklist] - The checklist items of the task. Optional.
 */

$(document).ready(() => {
    // @ts-ignore
    const vscode = acquireVsCodeApi();

    /**
     * Load the Kanban board data into the UI
     * @param {KanbanBoard} board 
     */
    function loadKanbanBoard(board) {
        const color = board.properties.color;
        $('#background-color-picker').val(color);
        $('body').css('background-color', color);
        $('#kanban-title').text(board.name);

        const $board = $('#board').empty();
        board.lists.forEach((list, index) => {
            const $list = createListElement(list, index + 1);
            $board.append($list);
        });

        const $addListButton = $('<button>').attr('id', 'add-list').text('Add another list +');
        $board.append($addListButton);
        bindAddListButtonEvent($addListButton);
    };

    window.addEventListener('message', (event) => {
        const { type, text: { json: kanbanBoard } } = event.data;
        if (type === 'update') {
            loadKanbanBoard(kanbanBoard);
            vscode.setState({ json: kanbanBoard });
        }
    });

    // Webviews are normally torn down when not visible and re-created when they become visible again.
    // State lets us save information across these re-loads
    const state = vscode.getState();
    if (state) {
        loadKanbanBoard(state.json);
    }

    function createListElement(list, listIndex) {
        const $list = $('<div>').addClass('list').attr('id', `list-${listIndex}`).data('name', list.name);
        const $listTitle = $('<input>').addClass('list-title').attr('placeholder', 'Enter list title').val(list.name);

        let previousTitle = $listTitle.val();
        $listTitle.on('blur', function () {
            const newTitle = $(this).val().trim();
            if (newTitle) {
                vscode.postMessage({
                    type: 'update',
                    path: `list[${previousTitle}].name`,
                    value: newTitle
                });
                $list.data('name', newTitle);
                previousTitle = newTitle;
            }
        }).on('keypress', (e) => {
            if (e.which === 13) $(this).blur();
        });

        const $cards = $('<div>').addClass('cards').sortable({
            connectWith: '.cards',
            tolerance: 'pointer',
            start: function (event, ui) {
                $(ui.item).data("parentName", ui.item.parent().parent().data('name'));
                $(ui.item).data("startIndex", ui.item.index());
            },
            stop: function (event, ui) {
                handleCardSortStop(ui);
            }
        });

        list.tasks.forEach(task => {
            const $card = createCardElement(task);
            $cards.append($card);
        });

        const $addCardButton = createAddCardButton($listTitle, $cards);
        const $listActionsButton = createListActionsButton($listTitle, $list);

        $list.append($listTitle, $listActionsButton, $cards, $addCardButton);
        return $list;
    };

    function handleCardSortStop(ui) {
        const $item = ui.item;
        const parentName = $item.data("parentName");
        const newParentName = $item.parent().parent().data('name');
        const newIndex = $item.index();

        if (parentName !== newParentName) {
            vscode.postMessage({
                type: 'move',
                path: `list[${parentName}].tasks[${$item.data('name')}]`,
                value: {
                    index: newIndex,
                    destination: `list[${newParentName}].tasks`
                }
            });
        } else {
            const startIndex = $item.data("startIndex");
            if (newIndex !== startIndex) {
                vscode.postMessage({
                    type: 'move',
                    path: `list[${parentName}].tasks[${$item.data('name')}]`,
                    value: {
                        index: newIndex
                    }
                });
            }
        }
    };

    function createCardElement(task) {
        const $card = $('<div>').addClass('card').data('name', task.name);
        const $cardTitleInput = $('<input>').addClass('card-title').attr('placeholder', 'Enter card title').val(task.name).prop('readonly', true);
        const $cardMenuButton = createCardMenuButton($card);
        const $cardMenuActions = createCardMenuActions($cardTitleInput);

        $card.append($cardTitleInput, $cardMenuButton, $cardMenuActions);
        return $card;
    };

    function createCardMenuButton($card) {
        return $('<button>').addClass('card-menu').text('⋮').on('click', (event) => {
            event.stopPropagation();
            closeAllMenus();
            $card.find('.card-menu-actions').toggle();
        });
    };

    function createCardMenuActions($cardTitleInput) {
        const $cardMenuActions = $('<div>').addClass('card-menu-actions').hide();
        const $editCardButton = $('<button>').addClass('edit-card').text('Edit').on('click', (event) => {
            event.stopPropagation();
            editCard($cardTitleInput);
        });

        $cardMenuActions.append($editCardButton);
        return $cardMenuActions;
    };

    function createAddCardButton($listTitle, $cards) {
        return $('<button>').addClass('add-card').text('Add another card +').on('click', function () {
            $(this).hide();
            const $card = createEditableCard($listTitle);
            $cards.append($card);
            $card.find('.card-title').trigger('focus');
        });
    };

    function createEditableCard($listTitle) {
        const $card = $('<div>').addClass('card');
        const $cardTitleInput = $('<input>').addClass('card-title').attr('placeholder', 'Enter card title');
        let isCardTitleChanged = false;

        $cardTitleInput.on('input', () => isCardTitleChanged = true).on('blur', function () {
            if (!isCardTitleChanged) {
                $card.remove();
            } else {
                $(this).prop('readonly', true);
                vscode.postMessage({
                    type: 'create',
                    path: `list[${$listTitle.val()}].tasks`,
                    value: {
                        name: $(this).val(),
                        description: '',
                        labels: [],
                        attachments: [],
                        checklist: []
                    }
                });
            }
            $('.add-card').show();
        }).on('keypress', (e) => {
            if (e.which === 13) $(this).blur();
        });

        const $cardMenuButton = createCardMenuButton($card);
        const $cardMenuActions = createCardMenuActions($cardTitleInput);

        $card.append($cardTitleInput, $cardMenuButton, $cardMenuActions);
        return $card;
    };

    function createListActionsButton($listTitle, $list) {
        const $listActionsButton = $('<button>').addClass('list-actions').text('⋮').on('click', (event) => {
            event.stopPropagation();
            closeAllMenus();
            $list.find('.list-actions-menu').toggle();
        });

        const $listActionsMenu = $('<div>').addClass('list-actions-menu').hide();
        const $removeListButton = $('<button>').addClass('remove-list').text('Remove list').on('click', () => {
            vscode.postMessage({
                type: 'delete',
                path: `list[${$listTitle.val()}]`
            });
            $list.remove();
        });

        $listActionsMenu.append($removeListButton);
        $listActionsButton.append($listActionsMenu);
        return $listActionsButton;
    };

    function bindAddListButtonEvent($addListButton) {
        $addListButton.on('click', function () {
            $(this).hide();
            const $board = $('#board');
            const $list = createNewList($board);
            $board.children('#add-list').before($list);
            $list.find('.list-title').trigger('focus');
        });
    };

    function createNewList($board) {
        const listId = `list-${$board.find('.list').length + 1}`;
        const $list = $('<div>').addClass('list').attr('id', listId);
        const $listTitle = $('<input>').addClass('list-title').attr('placeholder', 'Enter list title');
        let isListTitleChanged = false;

        $listTitle.on('input', () => isListTitleChanged = true).on('blur', function () {
            if (!isListTitleChanged) {
                $list.remove();
            } else {
                const listName = $(this).val();
                vscode.postMessage({
                    type: 'create',
                    path: 'list',
                    value: { name: listName }
                });
                $list.data('name', listName);
            }
            $('#add-list').show();
            $list.find('.add-card').show();
            $list.find('.cards').show();
        }).on('keypress', (e) => {
            if (e.which === 13) $(this).blur();
        });

        const $cards = $('<div>').addClass('cards').sortable({
            connectWith: '.cards',
            tolerance: 'pointer',
            start: (event, ui) => {
                $(ui.item).data("parentName", ui.item.parent().parent().data('name'));
                $(ui.item).data("startIndex", ui.item.index());
            },
            stop: (event, ui) => {
                handleCardSortStop(ui);
            }
        }).hide();

        const $addCardButton = createAddCardButton($listTitle, $cards);
        const $listActionsButton = createListActionsButton($listTitle, $list);

        $list.append($listTitle, $listActionsButton, $cards, $addCardButton);
        return $list;
    };

    function editCard($cardTitleInput) {
        $('#edit-card-title').val($cardTitleInput.val());
        $('#edit-card-description').val('');
        $('#card-modal').show();
        $cardTitleInput.prop('readonly', false).focus();

        $('#save-card').one('click', () => {
            $cardTitleInput.val($('#edit-card-title').val());
            $('#card-modal').hide();
        });
    };

    function closeAllMenus() {
        $('.card-menu-actions, .list-actions-menu').hide();
    };

    $('#background-color-picker').on('input', (event) => {
        const color = event.target.value;
        $('body').css('background-color', color);
        vscode.postMessage({
            type: 'update',
            path: 'color',
            value: color
        });
    });

    const $modal = $('#card-modal');
    $('.close').on('click', () => $modal.hide());

    $(window).on('click', (event) => {
        if ($(event.target).is($modal)) $modal.hide();
        closeAllMenus();
    });

    $('#save-card').on('click', () => {
        const cardTitle = $('#edit-card-title').val();

        $('.card').each(function () {
            const $cardTitleInput = $(this).find('.card-title');
            if ($cardTitleInput.val() === cardTitle) {
                $cardTitleInput.val(cardTitle);
            }
        });

        $('.list').each(function () {
            const $listTitleInput = $(this).find('.list-title');
            if ($listTitleInput.val() === cardTitle) {
                $listTitleInput.val(cardTitle);
            }
        });

        $modal.hide();
    });

    const $kanbanTitle = $('#kanban-title');
    const $editTitleInput = $('#edit-title-input');

    $kanbanTitle.on('click', () => {
        $kanbanTitle.hide();
        $editTitleInput.show().val($kanbanTitle.text().trim()).trigger('focus');
    });

    $editTitleInput.on('blur', () => {
        const newTitle = $editTitleInput.val().trim();
        if (newTitle === '') {
            $editTitleInput.val($kanbanTitle.text().trim());
        } else {
            $kanbanTitle.text(newTitle);
            vscode.postMessage({
                type: 'update',
                path: 'name',
                value: newTitle
            });
        }
        $kanbanTitle.show();
        $editTitleInput.hide();
    });

    $('#board').sortable({
        items: '> .list',
        tolerance: 'pointer',
        start: function (event, ui) {
            $(ui.item).data("startIndex", ui.item.index());
        },
        stop: function (event, ui) {
            const $item = ui.item;
            const startIndex = $item.data("startIndex");
            const newIndex = $item.index();
            if (newIndex !== startIndex) {
                vscode.postMessage({
                    type: 'move',
                    path: `list[${$item.data('name')}]`,
                    value: {
                        index: newIndex,
                    }
                });
            }
        }
    });
});
