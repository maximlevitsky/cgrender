/*
	This file is part of CG4.

	Copyright (c) Inbar Donag and Maxim Levitsky

    CG4 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    CG4 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with CG4.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "ActionButton.h"

ActionButton::ActionButton(QWidget *parent) :
    QPushButton(parent)
{
    actionOwner = NULL;
    setCheckable(true);
}

void ActionButton::setAction(QAction *action)
{

    // if I've got already an action associated to the button
    // remove all connections

    if( actionOwner != NULL && actionOwner != action ){
        disconnect( actionOwner,
                    SIGNAL(changed()),
                    this,
                    SLOT(updateButtonStatusFromAction()) );

        disconnect( this,
                    SIGNAL(clicked()),
                    actionOwner,
                    SLOT(trigger()) );
    }


    // store the action
    actionOwner = action;

    // configure the button
    updateButtonStatusFromAction();



    // connect the action and the button
    // so that when the action is changed the
    // button is changed too!
    connect( action,
             SIGNAL(changed()),
             this,
             SLOT(updateButtonStatusFromAction()));


    // connect the button to the slot that forwards the
    // signal to the action
    connect( this,
             SIGNAL(clicked()),
             actionOwner,
             SLOT(trigger()) );
}

void ActionButton::updateButtonStatusFromAction()
{
    setText( actionOwner->text() );
    setStatusTip( actionOwner->statusTip() );
    setToolTip( actionOwner->toolTip() );
    setIcon( actionOwner->icon() );
    setEnabled( actionOwner->isEnabled() );
    setChecked(actionOwner->isChecked());

}
