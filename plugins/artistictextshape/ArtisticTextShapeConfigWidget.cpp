/* This file is part of the KDE project
 * Copyright (C) 2007,2011 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2008 Rob Buis <buis@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "ArtisticTextShapeConfigWidget.h"
#include "ArtisticTextTool.h"
#include "ArtisticTextToolSelection.h"
#include "ArtisticTextShape.h"
#include "ChangeTextAnchorCommand.h"
#include "ChangeTextFontCommand.h"

#include <QtGui/QButtonGroup>

#include <KoCanvasController.h>
#include <KoToolManager.h>
#include <KoShapeManager.h>
#include <KoCanvasBase.h>

ArtisticTextShapeConfigWidget::ArtisticTextShapeConfigWidget(ArtisticTextTool *textTool)
    : m_textTool(textTool), m_anchorGroup(0)
{
    Q_ASSERT(m_textTool);

    widget.setupUi( this );

    widget.bold->setCheckable( true );
    widget.bold->setIcon( KIcon( "format-text-bold" ) );
    widget.italic->setCheckable( true );
    widget.italic->setIcon( KIcon( "format-text-italic" ) );
    widget.anchorStart->setIcon( KIcon( "format-justify-left" ) );
    widget.anchorStart->setCheckable( true );
    widget.anchorMiddle->setIcon( KIcon( "format-justify-center" ) );
    widget.anchorMiddle->setCheckable( true );
    widget.anchorEnd->setIcon( KIcon( "format-justify-right" ) );
    widget.anchorEnd->setCheckable( true );
    widget.fontSize->setRange( 2, 1000 );

    m_anchorGroup = new QButtonGroup(this);
    m_anchorGroup->addButton( widget.anchorStart );
    m_anchorGroup->addButton( widget.anchorMiddle );
    m_anchorGroup->addButton( widget.anchorEnd );

    connect( widget.fontFamily, SIGNAL(currentFontChanged(const QFont&)), this, SLOT(propertyChanged()));
    connect( widget.fontSize, SIGNAL(valueChanged(int)), this, SLOT(propertyChanged()));
    connect( widget.bold, SIGNAL(toggled(bool)), this, SLOT(propertyChanged()));
    connect( widget.italic, SIGNAL(toggled(bool)), this, SLOT(propertyChanged()));
    connect( m_anchorGroup, SIGNAL(buttonClicked(int)), this, SLOT(propertyChanged()));
}

void ArtisticTextShapeConfigWidget::blockChildSignals( bool block )
{
    widget.fontFamily->blockSignals( block );
    widget.fontSize->blockSignals( block );
    widget.bold->blockSignals( block );
    widget.italic->blockSignals( block );
    m_anchorGroup->blockSignals( block );
}

void ArtisticTextShapeConfigWidget::propertyChanged()
{
    ArtisticTextToolSelection *selection = dynamic_cast<ArtisticTextToolSelection*>(m_textTool->selection());
    if (!selection)
        return;

    ArtisticTextShape *currentText = selection->selectedShape();
    if (!currentText)
        return;

    QFont font = currentText->fontAt(m_textTool->textCursor());
    font.setFamily( widget.fontFamily->currentFont().family() );
    font.setBold( widget.bold->isChecked() );
    font.setItalic( widget.italic->isChecked() );
    font.setPointSize( widget.fontSize->value() );

    ArtisticTextShape::TextAnchor newAnchor;
    if ( widget.anchorStart->isChecked() )
        newAnchor = ArtisticTextShape::AnchorStart;
    else if ( widget.anchorMiddle->isChecked() )
        newAnchor = ArtisticTextShape::AnchorMiddle;
    else
        newAnchor = ArtisticTextShape::AnchorEnd;

    QUndoCommand * cmd = 0;
    if ( newAnchor != currentText->textAnchor() ) {
        cmd = new ChangeTextAnchorCommand(currentText, newAnchor);
    }
    else if( font.key() != currentText->fontAt(m_textTool->textCursor()).key() ) {
        if (selection->hasSelection()) {
            cmd = new ChangeTextFontCommand(currentText, selection->selectionStart(), selection->selectionCount(), font);
        }
    }

    if( cmd )
        m_textTool->canvas()->addCommand(cmd);
}

void ArtisticTextShapeConfigWidget::updateWidget()
{
    ArtisticTextToolSelection *selection = dynamic_cast<ArtisticTextToolSelection*>(m_textTool->selection());
    if (!selection)
        return;

    ArtisticTextShape *currentText = selection->selectedShape();
    if (!currentText)
        return;

    blockChildSignals( true );

    QFont font = currentText->fontAt(m_textTool->textCursor());

    widget.fontSize->setValue( font.pointSize() );
    font.setPointSize( 8 );

    widget.fontFamily->setCurrentFont( font );
    widget.bold->setChecked( font.bold() );
    widget.italic->setChecked( font.italic() );
    if( currentText->textAnchor() == ArtisticTextShape::AnchorStart )
        widget.anchorStart->setChecked( true );
    else if( currentText->textAnchor() == ArtisticTextShape::AnchorMiddle )
        widget.anchorMiddle->setChecked( true );
    else
        widget.anchorEnd->setChecked( true );

    blockChildSignals( false );
}
