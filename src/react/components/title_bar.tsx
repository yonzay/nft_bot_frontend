/* eslint-disable @typescript-eslint/no-explicit-any */
import React, { useState } from 'react';
import settings_img from '../assets/settings.png';
import minimize_img from '../assets/minimize.png';
import maximize_restore from '../assets/maximize_restore.png';
import exit_img from '../assets/exit.png';

enum modes { unauthenticated = 'unauthenticated', main = 'main', modal = 'modal' }

type mode = 'main' | 'modal'

interface ClientInformation {
    user_info: string;
    user_info_email: string;
    status_indicator_color: string;
    status_info: string;
    version: string;
}

class TitleBar {
    private static content: ClientInformation | undefined;
    public static update_content: React.Dispatch<React.SetStateAction<ClientInformation | undefined>>;
    private static settings = (): void => {
        (window as any).electron.ipcRenderer.message('perform', ['open', 'settings', null]);
    };
    private static minimize = (id: string): void => {
        (window as any).electron.ipcRenderer.message('perform', ['minimize', id, null]);
    };
    private static maximize_restore = (id: string): void => {
        (window as any).electron.ipcRenderer.message('perform', ['maximize', id, null]);
    };
    private static exit = (id: string): void => {
        (window as any).electron.ipcRenderer.message('perform', ['close', id, null]);
    };
    public static Make: React.FC<{ id: string, mode: mode, title: string }> = ({ id, mode, title }) => {
        if (mode === modes.main) {
            [this.content, this.update_content] = useState<ClientInformation>();
            return (
                <div className='d-flex flex-row align-items-center justify-content-between title-bar unselectable'>
                    <div className='d-flex flex-row align-items-center ms-0 unselectable'>
                        <p className='semi-bold text-white text-sm ms-2 mb-0 unselectable'>{ title }</p>
                    </div>
                    <div className='d-flex flex-row align-items-center unselectable'>
                        <p className='semi-bold text-white text-xs mb-0 border-right unselectable'>{ this.content?.user_info }</p>
                        <div className='short-line unselectable'/>
                        <p className='semi-bold text-white text-xs mb-0 border-right no-drag unselectable'><span className='add-underline unselectable' onClick={ () => { (window as any).electron.ipcRenderer.message('perform', ['href', 'main', { email: this.content?.user_info_email } ]) } }>{ this.content?.user_info_email }</span></p>
                        <div className='short-line unselectable'/>
                        <div className='ms-2 me-1 unselectable' style={{
                            width: '5px',
                            height: '5px',
                            borderRadius: '50%',
                            backgroundColor: `${ this.content?.status_indicator_color }`,
                            marginBottom: '.1em'
                        }}/>
                        <p className='semi-bold text-white text-xs mb-0 border-right ps-0 unselectable'>{ this.content?.status_info }</p>
                        <div className='short-line unselectable'/>
                        <p className='semi-bold text-white text-xs mb-0 border-right unselectable'>{ this.content?.version }</p>
                        <div className='short-line unselectable'/>
                        <div className='title-bar-icon unselectable' onClick={ this.settings }>
                            <img className='unselectable' src={ settings_img } alt=''/>
                        </div>
                        <div className='title-bar-icon unselectable' onClick={ () => { this.minimize(id) } }>
                            <img className='unselectable' src={ minimize_img } alt=''/>
                        </div>
                        <div className='title-bar-icon title-bar-icon-x unselectable' onClick={ () => { this.exit(id) } }>
                            <img className='unselectable' src={ exit_img } alt=''/>
                        </div>
                    </div>
                </div>
            );
        } else if (mode === modes.modal) {
            return (
                <div className='d-flex flex-row align-items-center justify-content-between title-bar unselectable'>
                    <div className='d-flex flex-row align-items-center ms-0 unselectable'>
                        <p className='semi-bold text-white text-sm ms-2 mb-0 unselectable'>{ title }</p>
                    </div>
                    <div className='d-flex flex-row align-items-center unselectable'>
                        <div className='title-bar-icon unselectable' onClick={ () => { this.minimize(id) } }>
                            <img className='unselectable' src={ minimize_img } alt=''/>
                        </div>
                        <div className='title-bar-icon unselectable' onClick={ () => { this.maximize_restore(id) } }>
                            <img className='unselectable' src={ maximize_restore } alt=''/>
                        </div>
                        <div className='title-bar-icon title-bar-icon-x unselectable' onClick={ () => { this.exit(id) } }>
                            <img className='unselectable' src={ exit_img } alt=''/>
                        </div>
                    </div>
                </div>
            );
        }
    };
}

export { TitleBar }