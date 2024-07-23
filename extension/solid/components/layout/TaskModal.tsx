import styles from './TaskModal.module.css';

import { createSignal, Show } from "solid-js";
import type { Component, Setter } from 'solid-js';
import { KanbanMarkdown } from "../../types";
import { LabelMenu } from './modal/LabelMenu';

type TaskModalProps = {
    kanban_board: KanbanMarkdown.KanbanBoard;
    setTaskModalState: Setter<boolean>;
};

export const TaskModal: Component<TaskModalProps> = (props) => {
    const { kanban_board, setTaskModalState } = props;

    const [getLabelMenuState, setLabelMenuState] = createSignal<boolean>(false);
    const [getAttachmentMenuState, setAttachmentMenuState] = createSignal<boolean>(false);

    const [getLabelMenuReference, setLabelMenuReference] = createSignal<HTMLDivElement>();
    let attachment_menu_reference: HTMLDivElement;

    function positionMenu(menu, button) {
        const buttonRect = button.getBoundingClientRect();
        const buttonHeight = button.offsetHeight;
        const buttonOffsetTop = buttonRect.top + window.scrollY;
        const buttonOffsetLeft = buttonRect.left + window.scrollX;

        menu.style.position = 'absolute';
        menu.style.top = `${buttonOffsetTop + buttonHeight}px`;
        menu.style.left = `${buttonOffsetLeft}px`;
        menu.style.display = 'flex';
        menu.style.flexDirection = 'column'; // Added to make the menu vertical
    }

    return (
        <div class={styles.modal}>
            <div id={styles.modal_content}>
                <span id={styles.modal_close} onClick={() => {
                    setTaskModalState(false);
                }}>&times;</span>
                <div id={styles.modal_main}>
                    <div id={styles.modal_header}>
                        <h2>Edit Card</h2>
                        <div id={styles.modal_label_bar}></div>
                    </div>
                    <div id={styles.modal_body}>
                        <label for="modal_edit_card_title">Title</label>
                        <textarea id={styles.modal_edit_card_title} placeholder="Enter card title"></textarea>
                        <label for="modal_edit_card_description">Description</label>
                        <textarea id={styles.modal_edit_card_description} placeholder="Enter card description"></textarea>
                    </div>
                    <div id={styles.modal_footer}>
                        <button id={styles.modal_save_card} onClick={() => {
                            // TODO: Save card
                            setTaskModalState(false);
                        }}>Save</button>
                    </div>
                </div>
                <div id={styles.modal_sidebar}>
                    <button id={styles.modal_label_button}
                        onClick={(event) => {
                            event.stopPropagation();
                            setLabelMenuState(b => !b);
                            if (getLabelMenuState()) {
                                setAttachmentMenuState(false);
                                positionMenu(getLabelMenuReference(), event.target);
                            }
                        }}>Labels</button>
                    <button id={styles.modal_attachment_button}
                        onClick={(event) => {
                            event.stopPropagation();
                            setAttachmentMenuState(b => !b);
                            if (getAttachmentMenuState()) {
                                setLabelMenuState(false);
                                positionMenu(attachment_menu_reference, event.target);
                            }
                        }}>Attachments</button>
                </div>
            </div>
            <Show when={getLabelMenuState()}>
                <LabelMenu setLabelMenuReference={setLabelMenuReference} />
            </Show>
            <Show when={getAttachmentMenuState()}>
                <div ref={attachment_menu_reference} class={styles.menu}>Attachment menu content</div>
            </Show>
        </div>
    );
}
