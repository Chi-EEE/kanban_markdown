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
 * @property {string} color - The color of the label.
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

    /**
     * Load the Kanban board data into the UI
     * @param {KanbanBoard} board 
     */
    function loadKanbanBoard(board) {
        const color = board.properties.color;
        $('#background-color-picker').val(color);
        $('body').css('background-color', color);
        $('.modal-content').css('background-color', color);
        $('#kanban-title').text(board.name);

        const $board = $('#board').empty();
        board.lists.forEach(function (list, index) {
            const $list = createListElement(list, index + 1);
            $board.append($list);
        });

        const $addListButton = $('<button>').attr('id', 'add-list').text('Add another list +');
        $board.append($addListButton);
        bindAddListButtonEvent($addListButton);
    };

    window.addEventListener('message', function (event) {
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
            // @ts-ignore
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
        }).on('keypress', function (e) {
            if (e.which === 13) $(this).trigger('blur');
        });

        const $cards = $('<div>').addClass('cards').sortable({
            connectWith: '.cards',
            tolerance: 'pointer',
            start: function (event, ui) {
                $(ui.item).data("parentName", ui.item.closest('.list').data('name'));
                $(ui.item).data("startIndex", ui.item.index());
            },
            stop: function (event, ui) {
                handleCardSortStop(ui);
            }
        });

        list.tasks.forEach(function (task) {
            const $card = createCardElement(task);
            $cards.append($card);
        });

        const $addCardButton = createAddCardButton($listTitle, $cards);
        const $listActionsButton = createListActionsButton($listTitle, $list);

        $list.append($listTitle, $listActionsButton, $cards, $addCardButton);
        return $list;
    };

    function handleCardSortStop(ui) {
        /** @type {JQuery<HTMLElement>} */
        const $item = ui.item;
        const parentName = $item.data("parentName");
        const newParentName = $item.closest('.list').data('name');
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

    /**
     * @param {Task} task 
     * @returns 
     */
    function createCardElement(task) {
        const $card = $('<div>').addClass('card')
            .data('name', task.name)
            .data('description', task.description)
            .data('checked', task.checked)
            .data('labels', task.labels || [])
            .data('attachments', task.attachments)
            .data('checklist', task.checklist);

        const $cardTitleInput = $('<input>').addClass('card-title').attr('placeholder', 'Enter card title').val(task.name).prop('readonly', true);
        const $cardMenuButton = createCardMenuButton($card);
        const $cardMenuActions = createCardMenuActions($card, $cardTitleInput);

        const $labelBar = $('<div>').addClass('label-bar');
        task.labels && task.labels.forEach(label => {
            const $label = $('<button>').addClass('label-button')
                .css('background-color', label.color)
                .text(label.name)
                .on('click', function () {
                    toggleLabelOnCard(label, $card, $labelBar);
                });
            $labelBar.append($label);
        });

        $card.append($cardTitleInput, $cardMenuButton, $cardMenuActions, $labelBar);
        return $card;
    };

    /**
     * Toggle the label on the card
     * @param {Label} label
     * @param {JQuery<HTMLElement>} $card
     * @param {JQuery<HTMLElement>} $label_bar
     */
    function toggleLabelOnCard(label, $card, $label_bar) {
        const labels = $card.data('labels') || [];
        const labelIndex = labels.findIndex(l => l.name === label.name);

        if (labelIndex === -1) {
            // Label is not selected, so add it
            labels.push(label);
            $('.modal-label-bar').append($('<button>').addClass('label-button')
                .css('background-color', label.color)
                .text(label.name))
                .on('click', function () {
                    const $currentCard = $card_modal.data('current-card');
                    if ($currentCard) {
                        toggleLabelOnCard(label, $currentCard, $currentCard.find('.label-bar'));
                    }
                    $('#label-menu').hide();
                });
            $label_bar.append($('<button>').addClass('label-button')
                .css('background-color', label.color)
                .text(label.name))
                .on('click', function () {
                    const $currentCard = $card_modal.data('current-card');
                    if ($currentCard) {
                        toggleLabelOnCard(label, $currentCard, $currentCard.find('.label-bar'));
                    }
                    $('#label-menu').hide();
                });
        } else {
            // Label is selected, so remove it
            labels.splice(labelIndex, 1);
            $('.modal-label-bar').find('.label-button').filter(function () {
                return $(this).text() === label.name;
            }).remove();
            $label_bar.find('.label-button').filter(function () {
                return $(this).text() === label.name;
            }).remove();
        }

        $card.data('labels', labels);
    }


    function createCardMenuButton($card) {
        return $('<button>').addClass('card-menu').text('⋮').on('click', function (event) {
            event.stopPropagation();
            closeAllMenus();
            $card.find('.card-menu-actions').toggle();
        });
    };

    /**
     * 
     * @param {JQuery<HTMLElement>} $card 
     * @param {*} $cardTitleInput 
     * @returns 
     */
    function createCardMenuActions($card, $cardTitleInput) {
        const $cardMenuActions = $('<div>').addClass('card-menu-actions').hide();
        const $editCardButton = $('<button>').addClass('edit-card').text('Edit').on('click', function (event) {
            event.stopPropagation();
            editCard($card, $cardTitleInput);
        });

        const $deleteCardButton = $('<button>').addClass('delete-card').text('Delete').on('click', function (event) {
            event.stopPropagation();
            vscode.postMessage({
                type: 'delete',
                path: `list[${$card.closest('.list').data('name')}].tasks[${$card.data('name')}]`
            });
            $card.remove();
        });

        $cardMenuActions.append($editCardButton, $deleteCardButton);
        return $cardMenuActions;
    }

    function createAddCardButton($listTitle, $cards) {
        return $('<button>').addClass('add-card').text('Add another card +').on('click', function () {
            $(this).hide();
            const $card = createEditableCard($listTitle);
            $cards.append($card);
            $card.find('.card-title').trigger('focus');
        });
    };

    function createEditableCard($listTitle) {
        const $card = $('<div>').addClass('card')
            .data('name', '')
            .data('description', '')
            .data('checked', false)
            .data('labels', [])
            .data('attachments', [])
            .data('checklist', []);

        const $cardTitleInput = $('<input>').addClass('card-title').attr('placeholder', 'Enter card title');
        let isCardTitleChanged = false;

        $cardTitleInput.on('input', function () { isCardTitleChanged = true }).on('blur', function () {
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
        }).on('keypress', function (e) {
            if (e.which == 13) {
                $(this).trigger('blur');
            }
        });

        const $cardMenuButton = createCardMenuButton($card);
        const $cardMenuActions = createCardMenuActions($card, $cardTitleInput);

        $card.append($cardTitleInput, $cardMenuButton, $cardMenuActions);
        return $card;
    };

    function createListActionsButton($listTitle, $list) {
        const $listActionsButton = $('<button>').addClass('list-actions').text('⋮').on('click', function (event) {
            event.stopPropagation();
            closeAllMenus();
            $list.find('.list-actions-menu').toggle();
        });

        const $listActionsMenu = $('<div>').addClass('list-actions-menu').hide();
        const $removeListButton = $('<button>').addClass('remove-list').text('Remove list').on('click', function () {
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

        $listTitle.on('input', function () { isListTitleChanged = true }).on('blur', function () {
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
        }).on('keypress', function (e) {
            if (e.which === 13) $(this).trigger('blur');
        });

        const $cards = $('<div>').addClass('cards').sortable({
            connectWith: '.cards',
            tolerance: 'pointer',
            start: function (event, ui) {
                $(ui.item).data("parentName", ui.item.closest('.list').data('name'));
                $(ui.item).data("startIndex", ui.item.index());
            },
            stop: function (event, ui) {
                handleCardSortStop(ui);
            }
        }).hide();

        const $addCardButton = createAddCardButton($listTitle, $cards);
        const $listActionsButton = createListActionsButton($listTitle, $list);

        $list.append($listTitle, $listActionsButton, $cards, $addCardButton);
        return $list;
    };

    const $card_modal = $('#card-modal');

    /**
    * Set the current card being edited or selected
    * @param {JQuery<HTMLElement>} $card
    */
    function setCurrentCard($card) {
        $card_modal.data('current-card', $card);
    }

    /**
     * @param {JQuery<HTMLElement>} $card 
     * @param {*} $cardTitleInput 
     */
    function editCard($card, $cardTitleInput) {
        $('#edit-card-title').val($card.data('name'));
        $('#edit-card-description').val($card.data('description'));

        const $label_bar = $card.find('.label-bar')
        $label_bar.empty();
        $('.modal-label-bar').empty();

        setCurrentCard($card);

        // Populate the label bar with the labels of the current card
        const labels = $card.data('labels') || [];
        labels.forEach(function (label) {
            $label_bar.append($('<button>').addClass('label-button')
                .css('background-color', label.color)
                .text(label.name)
                .on('click', function () {
                    toggleLabelOnCard(label, $card, $label_bar);
                }));
            $('.modal-label-bar').append($('<button>').addClass('label-button')
                .css('background-color', label.color)
                .text(label.name)
                .on('click', function () {
                    toggleLabelOnCard(label, $card, $label_bar);
                }));
        });

        $card_modal.show();
        $cardTitleInput.prop('readonly', false).focus();

        $('#save-card').one('click', function () {
            $cardTitleInput.val($('#edit-card-title').val());
            $card_modal.hide();
        });
    }

    $('#background-color-picker').on('input', function (event) {
        // @ts-ignore
        const color = event.target.value;
        $('body').css('background-color', color);
        $('#modal-content').css('background-color', color);
        vscode.postMessage({
            type: 'update',
            path: 'color',
            value: color
        });
    });

    function positionMenu($menu, $button) {
        const buttonOffset = $button.offset();
        const buttonHeight = $button.outerHeight();
        $menu.css({
            position: 'absolute',
            top: buttonOffset.top + buttonHeight,
            left: buttonOffset.left,
            display: 'flex',
            'flex-direction': 'column' // Added to make the menu vertical
        }).show();
    }

    $('#label-button').on('click', function (event) {
        event.stopPropagation();
        closeAllMenus();
        positionMenu($('#label-menu'), $(this));
    });

    $('#attachment-button').on('click', function (event) {
        event.stopPropagation();
        closeAllMenus();
        positionMenu($('#attachment-menu'), $(this));
    });

    $('#create-label-button').on('click', function () {
        $('#label-select').hide();
        $('#label-create').show();
    });

    $('#back-to-label-select').on('click', function () {
        $('#label-create').hide();
        $('#label-select').show();
    });

    $('#create-label').on('click', function () {
        /** @type {string} */
        // @ts-ignore
        const title = $('#new-label-title').val();
        /** @type {string} */
        // @ts-ignore
        const color = $('#new-label-color').val();
        if (title && color) {
            vscode.postMessage({
                type: 'create',
                path: 'labels',
                value: { name: title, color: color }
            });

            const $newLabel = $('<button>')
                .addClass('label-button')
                .css('background-color', color)
                .text(title).on('click', function () {
                    const $currentCard = $card_modal.data('current-card');
                    if ($currentCard) {
                        toggleLabelOnCard({ name: title, color: color }, $currentCard, $currentCard.find('.label-bar'));
                    }
                    $('#label-menu').hide();
                });
            $('#label-list').append($newLabel);

            // Clear the form
            $('#new-label-title').val('');
            $('#new-label-color').val('#ffffff');

            // Switch back to label selection mode
            $('#label-create').hide();
            $('#label-select').show();
        }
    });

    function closeAllMenus() {
        $('.menu').hide();
        $('.card-menu-actions, .list-actions-menu').hide();
    }

    $('.close').on('click', function () {
        $card_modal.hide();
    });

    $(window).on('click', function () {
        closeAllMenus();
    });

    $(document).on('click', '.menu', function (event) {
        event.stopPropagation();
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

        $card_modal.hide();
    });

    const $kanbanTitle = $('#kanban-title');
    const $editTitleInput = $('#edit-title-input');

    $kanbanTitle.on('click', function () {
        $kanbanTitle.hide();
        $editTitleInput.show().val($kanbanTitle.text().trim()).trigger('focus');
    });

    $editTitleInput.on('blur', function () {
        // @ts-ignore
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
    }).on('keypress', function (e) {
        if (e.which === 13) $(this).trigger('blur');
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
