$(document).ready(function () {
    /**
     * Load the Kanban board data into the UI
     * @param {KanbanBoard} board 
     */
    function loadKanbanBoard(board) {
        const $board = $('#board').empty();
        board.lists.forEach(function (list, index) {
            const $list = createListElement(board, list, index + 1);
            $board.append($list);
            autoResize($list.find('.list-title'));
        });

        const $label_list = $('#modal-label-list').empty();
        console.log(board.labels)
        board.labels.forEach(function (label) {
            const $newLabel = $('<button>')
                .addClass('label-button')
                .css('background-color', label.color)
                .text(label.name).on('click', function () {
                    const $currentCard = $card_modal.data('current-card');
                    if ($currentCard) {
                        toggleLabelOnCard(label, $currentCard, $currentCard.find('.label-bar'));
                    }
                    $('#modal-label-menu').show();
                });
            $label_list.append($newLabel);
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

    /**
     * @param {JQuery<HTMLElement>} element 
     */
    function autoResize(element) {
        element.css('height', '0');
        element.css('height', element.prop('scrollHeight') + 'px');
    }

    $("textarea").each(function () {
        $(this).css('height', $(this).prop('scrollHeight') + 'px');
        $(this).css('overflow-y', 'hidden');
    }).on("input", function () {
        autoResize($(this))
    });

    /**
     * 
     * @param {KanbanBoard} board 
     * @param {List} list 
     * @param {number} listIndex 
     * @returns 
     */
    function createListElement(board, list, listIndex) {
        const $list = $('<div>').addClass('list').attr('id', `list-${listIndex}`)
            .data('name', list.name)
            .data('checked', list.checked);
        const $listTitle = $('<textarea>').addClass('list-title').attr('placeholder', 'Enter list title').val(list.name);

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
            const $card = createCardElement(board, task);
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
                path: `list[${parentName}].tasks[${$item.data('name')}][${$item.data('counter')}]`,
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
                    path: `list[${parentName}].tasks[${$item.data('name')}][${$item.data('counter')}]`,
                    value: {
                        index: newIndex
                    }
                });
            }
        }
    };

    /**
     * @param {KanbanBoard} board
     * @param {Task} task 
     * @returns 
     */
    function createCardElement(board, task) {
        const $card = $('<a>').addClass('card')
            .data('name', task.name)
            .data('counter', task.counter)
            .data('description', task.description)
            .data('checked', task.checked)
            .data('labels', task.labels)
            .data('attachments', task.attachments)
            .data('checklist', task.checklist);

        $card.on('click', function (event) {
            event.stopPropagation();
            editCard($card, $cardTitleInput);
        });

        const $cardTitleInput = $('<a>').addClass('card-title').attr('placeholder', 'Enter card title').text(task.name).prop('readonly', true).on('click', function (event) {
            event.stopPropagation();
            editCard($card, $cardTitleInput);
        });

        const $cardMenuActions = createCardMenuActions($card, $cardTitleInput);

        const $cardMenuButton = createCardMenuButton($card, $cardMenuActions);

        $card.on('mouseover', function () {
            $cardMenuButton.show();
        });

        $card.on('mouseleave', function () {
            $cardMenuButton.hide();
        });

        const $label_bar = $('<div>').addClass('label-bar');
        if (task.labels.length > 0) {
            task.labels.forEach(label => {
                const $label = $('<button>').addClass('label-button')
                    .css('background-color', label.color)
                    .text(label.name)
                    .on('click', function () {
                        toggleLabelOnCard(label, $card, $label_bar);
                        $('#modal-label-menu').show();
                    });
                $label_bar.append($label);
            });
            $label_bar.show();
        } else {
            $label_bar.hide();
        }

        $card.append($cardTitleInput, $cardMenuButton, $cardMenuActions, $label_bar);
        return $card;
    };

    /**
     * Toggle the label on the card
     * @param {Label} label
     * @param {JQuery<HTMLElement>} $card
     * @param {JQuery<HTMLElement>} $label_bar
     */
    function toggleLabelOnCard(label, $card, $label_bar) {
        console.log("aczxcxzasdfcjbnhsu: ", $card)
        console.log("aczxcxzasdfcjzxczxbnhsu: ", $card.data())
        console.log("acasdcxzzszxcxz: ", $card.data('labels'))
        const labels = $card.data('labels');
        console.log("aczxcxz", labels)
        console.log("aczxzxczxcxz: ", labels.length)
        const labelIndex = labels.findIndex(l => l.name === label.name);
        console.log("aczxcxzafsdojcuniadks: ", labelIndex)

        if (labelIndex === -1) {
            labels.push(label);
            $label_bar.show()
            addLabel(label, $card, $label_bar);
            vscode.postMessage({
                type: 'create',
                path: `list[${$card.closest('.list').data('name')}].tasks[${$card.data('name')}][${$card.data('counter')}].labels`,
                value: label
            });
        } else {
            // Label is selected, so remove it
            labels.splice(labelIndex, 1);
            $('#modal-label-bar').find('.label-button').filter(function () {
                return $(this).text() === label.name;
            }).remove();
            $label_bar.find('.label-button').filter(function () {
                return $(this).text() === label.name;
            }).remove();
            vscode.postMessage({
                type: 'delete',
                path: `list[${$card.closest('.list').data('name')}].tasks[${$card.data('name')}][${$card.data('counter')}].labels[${label.name}]`
            });
        }

        $card.data('labels', labels);
    }

    /**
     * 
     * @param {JQuery<HTMLElement>} $card 
     * @param {JQuery<HTMLElement>} $cardMenuActions 
     * @returns 
     */
    function createCardMenuButton($card, $cardMenuActions) {
        return $('<button>').addClass('card-menu').text('\u2710').on('click', function (event) {
            event.stopPropagation();
            $cardMenuActions.toggle();
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
            $cardMenuActions.hide();
        });

        const $deleteCardButton = $('<button>').addClass('delete-card').text('Delete').on('click', function (event) {
            event.stopPropagation();
            vscode.postMessage({
                type: 'delete',
                path: `list[${$card.closest('.list').data('name')}].tasks[${$card.data('name')}][${$card.data('counter')}]`
            });
            $card.remove();
            $cardMenuActions.hide();
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
        const $card = $('<a>').addClass('card')
            .data('name', '')
            .data('counter', 1)
            .data('description', '')
            .data('checked', false)
            .data('labels', [])
            .data('attachments', [])
            .data('checklist', []);

        $card.on('click', function (event) {
            event.stopPropagation();
            editCard($card, $cardTitleInput);
        });

        const $cardTitleInput = $('<textarea>').addClass('card-title').attr('placeholder', 'Enter card title');
        let isCardTitleChanged = false;

        $cardTitleInput.on('input', function () { isCardTitleChanged = true }).on('blur', function () {
            if (!isCardTitleChanged) {
                $card.remove();
            } else {
                $(this).prop('readonly', true);
                const name = $(this).val();
                vscode.postMessage({
                    type: 'create',
                    path: `list[${$listTitle.val()}].tasks`,
                    value: {
                        name: name,
                        description: '',
                        labels: [],
                        attachments: [],
                        checklist: []
                    }
                });
                $cardTitleInput.remove();
                // @ts-ignore
                $('a').addClass('card-title').text(name).on('click', function () {
                    $card.find('.card-title').show().trigger('focus');
                });
            }
            $('.add-card').show();
        }).on('keypress', function (e) {
            if (e.which == 13) {
                $(this).trigger('blur');
            }
        });

        const $cardMenuActions = createCardMenuActions($card, $cardTitleInput);

        const $cardMenuButton = createCardMenuButton($card, $cardMenuActions);

        $card.on('mouseover', function () {
            $cardMenuButton.show();
        });

        $card.on('mouseleave', function () {
            $cardMenuButton.hide();
        });


        $card.append($cardTitleInput, $cardMenuButton, $cardMenuActions);
        return $card;
    };

    function createListActionsButton($listTitle, $list) {
        const $listActionsMenu = $('<div>').addClass('list-actions-menu').hide();

        const $listActionsButton = $('<button>').addClass('list-actions').text('⋮').on('click', function (event) {
            event.stopPropagation();
            $listActionsMenu.toggle();
        });

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
        const $listTitle = $('<textarea>').addClass('list-title').attr('placeholder', 'Enter list title');
        let isListTitleChanged = false;

        $listTitle.on('input', function () { isListTitleChanged = true }).on('blur', function () {
            if (!isListTitleChanged) {
                $list.remove();
            } else {
                const listName = $(this).val();
                vscode.postMessage({
                    type: 'create',
                    path: 'list',
                    value: { checked: false, name: listName }
                });
                $list.data('name', listName);
                $listTitle.remove()
                // @ts-ignore
                $('h2').addClass('list-title').text(listName).on('click', function () {
                    $list.find('.list-title').show().trigger('focus');
                });
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
     * 
     * @param {Label} label 
     * @param {JQuery<HTMLElement>} $card 
     * @param {JQuery<HTMLElement>} $label_bar 
     */
    function addLabel(label, $card, $label_bar) {
        $label_bar.append($('<button>').addClass('label-button')
            .css('background-color', label.color)
            .text(label.name)
            .on('click', function () {
                toggleLabelOnCard(label, $card, $label_bar);
                $('#modal-label-menu').show();
            }));
        $('#modal-label-bar').append($('<button>').addClass('modal-label-button')
            .css('background-color', label.color)
            .text(label.name)
            .on('click', function () {
                toggleLabelOnCard(label, $card, $label_bar);
                $('#modal-label-menu').show();
            }));
        const labels = $card.data('labels');
        labels.concat(label)
        $card.data('labels', labels);
    }

    /**
     * @param {JQuery<HTMLElement>} $card 
     * @param {JQuery<HTMLElement>} $cardTitleInput 
     */
    function editCard($card, $cardTitleInput) {
        const $modal_edit_card_title = $('#modal-edit-card-title')
        const $modal_edit_card_description = $('#modal-edit-card-description')

        $modal_edit_card_title.val($card.data('name'));
        $modal_edit_card_description.val($card.data('description'));

        const $label_bar = $card.find('.label-bar')
        $label_bar.empty();
        $label_bar.hide();
        $('#modal-label-bar').empty();

        setCurrentCard($card);

        /** @type {Label[]} */
        const labels = $card.data('labels');
        if (labels.length > 0) {
            $label_bar.show();
            labels.forEach(function (label) {
                addLabel(label, $card, $label_bar)
            });
        }

        $card_modal.show();

        autoResize($modal_edit_card_title);
        autoResize($modal_edit_card_description);

        $('#modal-save-card').one('click', function () { saveCard($card, $cardTitleInput) });
    }

    /**
     * @param {JQuery<HTMLElement>} $card 
     * @param {JQuery<HTMLElement>} $cardTitleInput 
     */
    function saveCard($card, $cardTitleInput) {
        const cardTitle = $('#modal-edit-card-title').val();

        $cardTitleInput.val();
        $card_modal.hide();

        if (cardTitle !== $card.data('name')) {
            vscode.postMessage({
                type: 'update',
                path: `list[${$card.closest('.list').data('name')}].tasks[${$card.data('name')}][${$card.data('counter')}].name`,
                value: cardTitle
            });
            $card.data('name', cardTitle);
        }
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
        });
    }

    $('#modal-label-button').on('click', function (event) {
        event.stopPropagation();
        $('#modal-label-menu').toggle();
        if ($('#modal-label-menu').is(":visible")) {
            $('#modal-attachment-menu').hide();
            positionMenu($('#modal-label-menu'), $(this));
        }
    });

    $('#modal-attachment-button').on('click', function (event) {
        event.stopPropagation();
        $('#modal-attachment-menu').toggle();
        if ($('#modal-attachment-menu').is(":visible")) {
            $('#modal-label-menu').hide();
            positionMenu($('#modal-attachment-menu'), $(this));
        }
    });

    $('#modal-create-label-button').on('click', function () {
        $('#modal-label-select').hide();
        $('#modal-label-create').show();
    });

    $('#modal-back-to-label-select').on('click', function () {
        $('#modal-label-create').hide();
        $('#modal-label-select').show();
    });

    $('#modal-create-label').on('click', function () {
        /** @type {string} */
        // @ts-ignore
        const title = $('#modal-new-label-title').val();
        /** @type {string} */
        // @ts-ignore
        const color = $('#modal-new-label-color').val();
        if (title && color) {
            const label = { name: title, color: color, tasks: [] };

            vscode.postMessage({
                type: 'create',
                path: 'labels',
                value: label
            });

            const $currentCard = $card_modal.data('current-card');

            const $newLabel = $('<button>')
                .addClass('label-button')
                .css('background-color', color)
                .text(title).on('click', function () {
                    if ($currentCard) {
                        toggleLabelOnCard(label, $currentCard, $currentCard.find('.label-bar'));
                    }
                    $('#modal-label-menu').show();
                });

            $('#modal-label-list').append($newLabel);

            /** @type {JQuery<HTMLElement>} */
            const $label_bar = $currentCard.find('.label-bar')
            $label_bar.append($newLabel);
            $label_bar.show();

            const $list = $currentCard.closest('.list')

            vscode.postMessage({
                type: 'create',
                path: `list[${$list.data('name')}].tasks[${$currentCard.data('name')}].labels`,
                value: label
            });

            // Clear the form
            $('#new-label-title').val('');
            $('#new-label-color').val('#ffffff');

            // Switch back to label selection mode
            $('#modal-label-create').hide();
            $('#modal-label-select').show();
        }
    });

    function closeAllMenus() {
        $('.menu').hide();
        $('.card-menu-actions, .list-actions-menu').hide();
    }

    $('#modal-close').on('click', function () {
        $card_modal.hide();
    });

    $(window).on('click', function () {
        closeAllMenus();
    });

    $(document).on('click', '.menu', function (event) {
        event.stopPropagation();
    });

    $('#modal-save-card').on('click', function () {
        const cardTitle = $('#modal-edit-card-title').val();

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
