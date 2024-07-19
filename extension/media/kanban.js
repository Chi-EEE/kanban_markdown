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

    const pSBC=(p,c0,c1,l)=>{
        let r,g,b,P,f,t,h,i=parseInt,m=Math.round,a=typeof(c1)=="string";
        if(typeof(p)!="number"||p<-1||p>1||typeof(c0)!="string"||(c0[0]!='r'&&c0[0]!='#')||(c1&&!a))return null;
        if(!this.pSBCr)this.pSBCr=(d)=>{
            let n=d.length,x={};
            if(n>9){
                [r,g,b,a]=d=d.split(","),n=d.length;
                if(n<3||n>4)return null;
                x.r=i(r[3]=="a"?r.slice(5):r.slice(4)),x.g=i(g),x.b=i(b),x.a=a?parseFloat(a):-1
            }else{
                if(n==8||n==6||n<4)return null;
                if(n<6)d="#"+d[1]+d[1]+d[2]+d[2]+d[3]+d[3]+(n>4?d[4]+d[4]:"");
                d=i(d.slice(1),16);
                if(n==9||n==5)x.r=d>>24&255,x.g=d>>16&255,x.b=d>>8&255,x.a=m((d&255)/0.255)/1000;
                else x.r=d>>16,x.g=d>>8&255,x.b=d&255,x.a=-1
            }return x};
        h=c0.length>9,h=a?c1.length>9?true:c1=="c"?!h:false:h,f=this.pSBCr(c0),P=p<0,t=c1&&c1!="c"?this.pSBCr(c1):P?{r:0,g:0,b:0,a:-1}:{r:255,g:255,b:255,a:-1},p=P?p*-1:p,P=1-p;
        if(!f||!t)return null;
        if(l)r=m(P*f.r+p*t.r),g=m(P*f.g+p*t.g),b=m(P*f.b+p*t.b);
        else r=m((P*f.r**2+p*t.r**2)**0.5),g=m((P*f.g**2+p*t.g**2)**0.5),b=m((P*f.b**2+p*t.b**2)**0.5);
        a=f.a,t=t.a,f=a>=0||t>=0,a=f?a<0?t:t<0?a:a*P+t*p:0;
        if(h)return"rgb"+(f?"a(":"(")+r+","+g+","+b+(f?","+m(a*1000)/1000:"")+")";
        else return"#"+(4294967296+r*16777216+g*65536+b*256+(f?m(a*255):0)).toString(16).slice(1,f?undefined:-2)
    }

    /**
     * Load the Kanban board data into the UI
     * @param {KanbanBoard} board 
     */
    function loadKanbanBoard(board) {
        const color = board.properties.color;
        $('#background-color-picker').val(color);
        document.documentElement.style.setProperty('--background-color', color);
        document.documentElement.style.setProperty('--menu-background-color', pSBC(-0.4, color));

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
            $cardMenuActions.hide();
        });

        const $deleteCardButton = $('<button>').addClass('delete-card').text('Delete').on('click', function (event) {
            event.stopPropagation();
            vscode.postMessage({
                type: 'delete',
                path: `list[${$card.closest('.list').data('name')}].tasks[${$card.data('name')}]`
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
