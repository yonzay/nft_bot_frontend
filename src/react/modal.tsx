import React, { useRef } from 'react';
import { TitleBar } from './components/title_bar';
import Particle from 'react-tsparticles';
import { RecursivePartial, IOptions } from 'tsparticles';
import particle_config from './components/particles_config.json';
import ReactAce from 'react-ace';
import perform_action from './assets/perform_action.png';
import '../style/style.css';
import '../style/modal_style.css'
import '../style/chronicle_style.css';
import 'ace-builds/webpack-resolver';

type mode = 'modal'
type show = 'none' | 'block'

class Modal {
    private modal_id: string;
    private modal_title: string;
    private modal_mode: mode;
    private second_bar: show;
    private ace_mode: string;
    private theme: string;
    private read_only: boolean;
    private default_value: string;
    private call_function: (input: string) => void;
    public ace_reference: React.LegacyRef<ReactAce> | undefined;
    public is_view: string = undefined;
    constructor(is_view: string, modal_id: string, modal_title: string, modal_mode: mode, second_bar: show, ace_mode: string, theme: string, read_only: boolean, default_value: string, call_function: (input: string) => void) {
        this.is_view = is_view;
        this.modal_id = modal_id;
        this.modal_title = modal_title;
        this.modal_mode = modal_mode;
        this.second_bar = second_bar;
        this.ace_mode = ace_mode;
        this.theme = theme;
        this.read_only = read_only;
        this.default_value = default_value;
        this.call_function = call_function;
    }
    public Make: React.FC = () => {
        (this.ace_reference as React.MutableRefObject<ReactAce | undefined>) = useRef<ReactAce>();
        return (
            <>
                <TitleBar.Make id={ this.modal_id } mode={ this.modal_mode } title={ this.modal_title }/>
                <div className='flex-row align-items-center justify-content-start modal-window-icon-bar z-index unselectable' style={{ display: `${ this.second_bar }` }}>
                    <div className='text-sm text-white regular modal-utility-bar icon-hover unselectable' onClick={ () => { this.call_function((this.ace_reference as React.MutableRefObject<ReactAce>).current.editor.getSession().getValue()) } }>
                        <img className='unselectable' src={ perform_action } alt=''/>
                    </div>
                </div>
                <div className={ `${ this.is_view ?? this.theme.split('-')[1] }-window` }>
                    <Particle className='modal-particles unselectable' options={ particle_config as RecursivePartial<IOptions> }/>
                    <ReactAce
                        ref={ this.ace_reference }
                        mode={ this.ace_mode }
                        className={ this.theme + ' unselectable' }
                        showGutter={ false }
                        showPrintMargin={ false }
                        fontSize={ 13 }
                        defaultValue={`${ this.default_value }`}
                        readOnly={ this.read_only }
                    />
                </div>
            </>
        );
    };
}

export { Modal }