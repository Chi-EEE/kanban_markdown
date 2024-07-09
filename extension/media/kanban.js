$(document).ready(function () {
    // Add List button click event
    $('#add-list').on("click", function () {
        const $board = $('#board');

        const $newList = $('<div>').addClass('list');

        const $listTitle = $('<input>')
            .addClass('list-title')
            .attr('placeholder', 'Enter list title');

        let isListTitleChanged = false;
        $listTitle.on('input', function () {
            isListTitleChanged = true;
        });

        $listTitle.on('blur', function () {
            if (!isListTitleChanged) {
                $newList.remove();
            }
        });

        const $cards = $('<div>').addClass('cards');

        const $addCardButton = $('<button>')
            .addClass('add-card')
            .text('Add another card +')
            .on('click', function () {
                const $card = $('<div>').addClass('card');

                $card.on('click', function () {
                    if (isCardTitleChanged) {
                        $('#edit-card-title').val($cardTitleInput.val());
                        $('#edit-card-description').val('');
                        $('#card-modal').css('display', 'block');
                        $('#save-card').on('click', function () {
                            $cardTitleInput.val($('#edit-card-title').val());
                            $('#card-modal').css('display', 'none');
                        });
                    }
                });

                const $cardTitleInput = $('<input>')
                    .addClass('card-title')
                    .attr('placeholder', 'Enter card title');

                let isCardTitleChanged = false;
                $cardTitleInput.on('input', function () {
                    isCardTitleChanged = true;
                });

                $cardTitleInput.on('blur', function () {
                    if (!isCardTitleChanged) {
                        $card.remove();
                    } else {
                        $cardTitleInput.prop('readonly', true);
                    }
                });

                const $cardMenuButton = $('<button>')
                    .addClass('card-menu')
                    .text('⋮')
                    .on('click', function (event) {
                        event.stopPropagation();
                        closeAllMenus();
                        const $actionsMenu = $card.find('.card-menu-actions');
                        $actionsMenu.toggle();
                    });

                const $cardMenuActions = $('<div>')
                    .addClass('card-menu-actions')
                    .css('display', 'none');

                const $editCardButton = $('<button>')
                    .addClass('edit-card')
                    .text('Edit')
                    .on("click", function (event) {
                        event.stopPropagation();
                        $('#edit-card-title').val($cardTitleInput.val());
                        $('#edit-card-description').val('');
                        $('#card-modal').css('display', 'block');
                        $cardTitleInput.prop('readonly', false).focus();

                        // When Save button in modal is clicked
                        $('#save-card').one('click', function () {
                            $cardTitleInput.val($('#edit-card-title').val());
                            $('#card-modal').css('display', 'none');
                        });
                    });

                $cardMenuActions.append($editCardButton);

                $card.append($cardTitleInput, $cardMenuButton, $cardMenuActions);
                $cards.append($card);
                $cardTitleInput.trigger('focus');
            });

        const $listActionsButton = $('<button>')
            .addClass('list-actions')
            .text('⋮')
            .on("click", function (event) {
                event.stopPropagation();
                closeAllMenus();
                const $listActionsMenu = $newList.find('.list-actions-menu');
                $listActionsMenu.toggle();
            });

        const $listActionsMenu = $('<div>')
            .addClass('list-actions-menu')
            .css('display', 'none');

        const $removeListButton = $('<button>')
            .addClass('remove-list')
            .text('Remove list')
            .on('click', function () {
                $newList.remove();
            });

        $listActionsMenu.append($removeListButton);

        $newList.append($listTitle, $listActionsButton, $listActionsMenu, $cards, $addCardButton);
        $board.children('#add-list').before($newList);
        $listTitle.trigger('focus');
    });

    // Background color picker
    $('#background-color-picker').on('input', function (event) {
        $('body').css('background-color', event.target.value);
    });

    // Modal functionality
    const $modal = $('#card-modal');
    $('.close').on('click', function () {
        $modal.css('display', 'none');
    });

    $(window).on('click', function (event) {
        if ($(event.target).is($modal)) {
            $modal.css('display', 'none');
        }
        closeAllMenus();
    });

    function closeAllMenus() {
        $('.card-menu-actions, .list-actions-menu').css('display', 'none');
    }

    // Save card button in modal
    $('#save-card').on("click", function () {
        const cardTitle = $('#edit-card-title').val();

        $('.card').each(function () {
            const $card = $(this);
            const $cardTitleInput = $card.find('.card-title');
            if ($cardTitleInput.val() === cardTitle) {
                $cardTitleInput.val(cardTitle);
            }
        });

        $('.list').each(function () {
            const $list = $(this);
            const $listTitleInput = $list.find('.list-title');
            if ($listTitleInput.val() === cardTitle) {
                $listTitleInput.val(cardTitle);
            }
        });

        $modal.css('display', 'none');
    });

    // Edit kanban title
    const $kanbanTitle = $('#kanban-title');
    const $editTitleInput = $('#edit-title-input');

    $kanbanTitle.on("click",function () {
        $kanbanTitle.hide();
        $editTitleInput.show().val($kanbanTitle.text().trim()).focus();
    });

    $editTitleInput.on("blur", function () {
        const newTitle = $editTitleInput.val().trim();
        if (newTitle === '') {
            $editTitleInput.val($kanbanTitle.text().trim());
        } else {
            $kanbanTitle.text(newTitle);
        }
        $kanbanTitle.show();
        $editTitleInput.hide();
    });
});
