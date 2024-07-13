/**
 * @typedef {Object} KanbanBoard
 * @property {number} created - The timestamp when the kanban board was created.
 * @property {number} last_modified - The timestamp when the kanban board was last modified.
 * @property {number} version - The version of the kanban board.
 * @property {string} checksum - The checksum for the kanban board.
 * @property {string} name - The name of the kanban board.
 * @property {string} description - The description of the kanban board.
 * @property {Label[]} labels - The labels associated with the kanban board.
 * @property {List[]} lists - The lists on the kanban board.
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

$(document).ready(function () {
    // @ts-ignore
    const vscode = acquireVsCodeApi();

    window.addEventListener('message', event => {
        /** @type {KanbanBoard} */
        const kanban_board = event.data.text;
        loadKanbanBoard(kanban_board);
    });

    /**
     * @param {KanbanBoard} board 
     */
    function loadKanbanBoard(board) {
        const $board = $('#board').empty();

        board.lists.forEach((list, index) => {
            const $newList = createListElement(list, index + 1);
            $board.append($newList);
        });

        const $addListButton = $('<button>').attr('id', 'add-list').text('Add another list +');
        $board.append($addListButton);
        bindAddListButtonEvent($addListButton);
    }

    function createListElement(list, listIndex) {
        const $newList = $('<div>').addClass('list').attr('id', `list-${listIndex}`);

        const $listTitle = $('<input>')
            .addClass('list-title')
            .attr('placeholder', 'Enter list title')
            .val(list.name);

        const $cards = $('<div>').addClass('cards').sortable({
            connectWith: '.cards',
            tolerance: 'pointer'
        });

        list.tasks.forEach(task => {
            const $card = createCardElement(task);
            $cards.append($card);
        });

        const $addCardButton = createAddCardButton($cards);
        const $listActionsButton = createListActionsButton($newList);

        $newList.append($listTitle, $listActionsButton, $cards, $addCardButton);
        return $newList;
    }

    /**
     * @param {Task} task 
     * @returns 
     */
    function createCardElement(task) {
        const $card = $('<div>').addClass('card');

        const $cardTitleInput = $('<input>')
            .addClass('card-title')
            .attr('placeholder', 'Enter card title')
            .val(task.name)
            .prop('readonly', true);

        const $cardMenuButton = createCardMenuButton($card);
        const $cardMenuActions = createCardMenuActions($cardTitleInput);

        $card.append($cardTitleInput, $cardMenuButton, $cardMenuActions);
        return $card;
    }

    function createCardMenuButton($card) {
        return $('<button>')
            .addClass('card-menu')
            .text('⋮')
            .on('click', function (event) {
                event.stopPropagation();
                closeAllMenus();
                $card.find('.card-menu-actions').toggle();
            });
    }

    function createCardMenuActions($cardTitleInput) {
        const $cardMenuActions = $('<div>').addClass('card-menu-actions').hide();

        const $editCardButton = $('<button>')
            .addClass('edit-card')
            .text('Edit')
            .on('click', function (event) {
                event.stopPropagation();
                editCard($cardTitleInput);
            });

        $cardMenuActions.append($editCardButton);
        return $cardMenuActions;
    }

    function createAddCardButton($cards) {
        return $('<button>')
            .addClass('add-card')
            .text('Add another card +')
            .on('click', function () {
                $(this).hide();
                const $card = createEditableCard();
                $cards.append($card);
                $card.find('.card-title').trigger('focus');
            });
    }

    function createEditableCard() {
        const $card = $('<div>').addClass('card');
        const $cardTitleInput = $('<input>').addClass('card-title').attr('placeholder', 'Enter card title');
        let isCardTitleChanged = false;

        $cardTitleInput.on('input', () => isCardTitleChanged = true);
        $cardTitleInput.on('blur', function () {
            if (!isCardTitleChanged) {
                $card.remove();
            } else {
                $(this).prop('readonly', true);
            }
            $('.add-card').show();
        });

        $cardTitleInput.on('keypress', function (e) {
            if (e.which === 13) $(this).blur(); // Enter key
        });

        const $cardMenuButton = createCardMenuButton($card);
        const $cardMenuActions = createCardMenuActions($cardTitleInput);

        $card.append($cardTitleInput, $cardMenuButton, $cardMenuActions);
        return $card;
    }

    function createListActionsButton($newList) {
        const $listActionsButton = $('<button>')
            .addClass('list-actions')
            .text('⋮')
            .on('click', function (event) {
                event.stopPropagation();
                closeAllMenus();
                $newList.find('.list-actions-menu').toggle();
            });

        const $listActionsMenu = $('<div>').addClass('list-actions-menu').hide();
        const $removeListButton = $('<button>')
            .addClass('remove-list')
            .text('Remove list')
            .on('click', () => $newList.remove());

        $listActionsMenu.append($removeListButton);
        $listActionsButton.append($listActionsMenu);
        return $listActionsButton;
    }

    function bindAddListButtonEvent($addListButton) {
        $addListButton.on('click', function () {
            $(this).hide();
            const $board = $('#board');
            const $newList = createNewList($board);
            $board.children('#add-list').before($newList);
            $newList.find('.list-title').trigger('focus');
        });
    }

    function createNewList($board) {
        const listId = `list-${$board.find('.list').length + 1}`;
        const $newList = $('<div>').addClass('list').attr('id', listId);
        const $listTitle = $('<input>').addClass('list-title').attr('placeholder', 'Enter list title');
        let isListTitleChanged = false;

        $listTitle.on('input', () => isListTitleChanged = true);
        $listTitle.on('blur', function () {
            if (!isListTitleChanged) {
                $newList.remove();
            }
            $('#add-list').show();
            $newList.find('.add-card').show();
            $newList.find('.cards').show();
        });

        $listTitle.on('keypress', function (e) {
            if (e.which === 13) $(this).blur(); // Enter key
        });

        const $cards = $('<div>').addClass('cards').sortable({
            connectWith: '.cards',
            tolerance: 'pointer'
        }).hide();

        const $addCardButton = createAddCardButton($cards);
        const $listActionsButton = createListActionsButton($newList);

        $newList.append($listTitle, $listActionsButton, $cards, $addCardButton);
        return $newList;
    }

    function editCard($cardTitleInput) {
        $('#edit-card-title').val($cardTitleInput.val());
        $('#edit-card-description').val('');
        $('#card-modal').show();
        $cardTitleInput.prop('readonly', false).focus();

        $('#save-card').one('click', function () {
            $cardTitleInput.val($('#edit-card-title').val());
            $('#card-modal').hide();
        });
    }

    function closeAllMenus() {
        $('.card-menu-actions, .list-actions-menu').hide();
    }

    $('#background-color-picker').on('input', function (event) {
        $('body').css('background-color', event.target.value);
    });

    const $modal = $('#card-modal');
    $('.close').on('click', () => $modal.hide());

    $(window).on('click', function (event) {
        if ($(event.target).is($modal)) $modal.hide();
        closeAllMenus();
    });

    $('#save-card').on('click', function () {
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

    $kanbanTitle.on('click', function () {
        $kanbanTitle.hide();
        $editTitleInput.show().val($kanbanTitle.text().trim()).trigger('focus');
    });

    $editTitleInput.on('blur', function () {
        const newTitle = $editTitleInput.val().trim();
        if (newTitle === '') {
            $editTitleInput.val($kanbanTitle.text().trim());
        } else {
            $kanbanTitle.text(newTitle);
        }
        $kanbanTitle.show();
        $editTitleInput.hide();
    });

    $('#board').sortable({
        items: '> .list',
        tolerance: 'pointer'
    });
});
