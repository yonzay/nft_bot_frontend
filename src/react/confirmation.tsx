/* eslint-disable @typescript-eslint/no-explicit-any */
import React from 'react';
import exit_img from './assets/exit.png';

class Confirmation {
    public static exit = (identifier: string): void => {
        (window as any).electron.ipcRenderer.message('perform', ['close', `confirmation_${ identifier }`, null]);
    };
    public static Make: React.FC<{ identifier: string, message: string, confirm_data: [type: string, identifer: string]}> = (props) => {
        return (
            <div className='modal-window confirmation-window'>
                <div className='d-flex flex-row align-items-center justify-content-between title-bar unselectable'>
                    <div className='d-flex flex-row align-items-center ms-0 unselectable'>
                        <p className='semi-bold text-white text-sm ms-2 mb-0 unselectable'>Confirm</p>
                    </div>
                    <div className='d-flex flex-row align-items-center unselectable'>
                        <div className='title-bar-icon title-bar-icon-x unselectable' onClick={ () => { this.exit(props.identifier) } }>
                            <img className='unselectable' src={ exit_img } alt=''/>
                        </div>
                    </div>
                </div>
                <div className='align-items-center justify-content-center w-100 z-index p-3 unselectable'>
                    <h6 className='text-white z-index mb-2 text-md text-center unselectable'>{ props.message }</h6>
                    <div className='d-flex flex-row align-items-center justify-content-between w-100 unselectable confirmation-btns'>
                        <div className='transparent-btn text-sm text-white d-flex flex-row align-items-center unselectable' onClick={ () => { (window as any).electron.ipcRenderer.message('perform', ['confirm', `confirmation`, { type: props.confirm_data['0'], identifier: props.confirm_data['1'] }]); this.exit(props.identifier) } }>Delete</div>
                        <div className='transparent-btn text-sm text-white d-flex flex-row align-items-center cancel-button unselectable' onClick={ () => { this.exit(props.identifier) } }>Cancel</div>
                    </div>
                </div>
            </div>
        );
    };
}

export { Confirmation }