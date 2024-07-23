import styles from '../TaskModal.module.css';

import { createSignal, Show } from "solid-js";
import type { Component } from 'solid-js';
import { KanbanMarkdown } from "../../../types";

export const LabelMenu: Component = () => {
    const [getLabelMenuState, setLabelMenuState] = createSignal<string>("select");

    return (
        <div id="modal-label-menu" class="menu">
            <Show when={getLabelMenuState() === "select"}>
                <div id="modal-label-select">
                    <h3>Labels</h3>
                    <div id={styles.modal_label_list}></div>
                    <button id={styles.modal_create_label_button}
                        onClick={() => {
                            setLabelMenuState("create");
                        }}>Create a new label</button>
                </div>
            </Show>
            <Show when={getLabelMenuState() === "create"}>
                <div id={styles.modal_label_create}>
                    <h3>Create Label</h3>
                    <label for="modal-new-label-title">Title</label>
                    <input type="text" id="modal-new-label-title" />
                    <label for="modal-new-label-color">Color</label>
                    <input type="color" id="modal-new-label-color" value="#ffffff" />
                    <button id="modal-create-label">Create</button>
                    <button id="modal-back-to-label-select"
                        onClick={() => {
                            setLabelMenuState("select");
                        }}>Back</button>
                </div>
            </Show>
        </div>
    );
}
