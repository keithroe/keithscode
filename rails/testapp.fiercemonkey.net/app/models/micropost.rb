# == Schema Information
# Schema version: 20100701200151
#
# Table name: microposts
#
#  id         :integer         not null, primary key
#  content    :string(255)
#  user_id    :integer
#  created_at :datetime
#  updated_at :datetime
#

class Micropost < ActiveRecord::Base
  belongs_to :user

  validates_length_of :content, :maximum => 140
end
